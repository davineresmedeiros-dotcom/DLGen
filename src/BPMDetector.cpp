#include "BPMDetector.hpp"
#include <Geode/Geode.hpp>
#include <fmod.hpp>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <numeric>
#include <vector>

using namespace geode::prelude;

namespace {
constexpr int ENVELOPE_HZ = 100;
constexpr double MIN_BPM = 70.0;
constexpr double MAX_BPM = 220.0;

Result<std::vector<float>> decodeOnsets(std::filesystem::path const& path) {
    FMOD::System* system = nullptr;
    if (FMOD::System_Create(&system) != FMOD_OK || !system)
        return Err("FMOD could not create an analysis system");
    system->setOutput(FMOD_OUTPUTTYPE_NOSOUND_NRT);
    if (system->init(1, FMOD_INIT_NORMAL, nullptr) != FMOD_OK) {
        system->release();
        return Err("FMOD analysis initialization failed");
    }
    FMOD::Sound* sound = nullptr;
    auto cleanup = [&]() {
        if (sound) sound->release();
        system->close();
        system->release();
    };
    auto pathText = utils::string::pathToString(path);
    if (system->createSound(pathText.c_str(),
        FMOD_OPENONLY | FMOD_ACCURATETIME | FMOD_CREATESTREAM, nullptr, &sound) != FMOD_OK || !sound) {
        cleanup();
        return Err("Could not open selected song for BPM analysis");
    }

    FMOD_SOUND_FORMAT format = FMOD_SOUND_FORMAT_NONE;
    int channels = 0, bits = 0;
    float frequency = 44100.f;
    sound->getFormat(nullptr, &format, &channels, &bits);
    sound->getDefaults(&frequency, nullptr);
    if (channels <= 0 || bits <= 0 || frequency <= 0.f) {
        cleanup();
        return Err("Unsupported song audio format");
    }
    const size_t bytesPerSample = static_cast<size_t>(bits / 8);
    const size_t frameBytes = bytesPerSample * static_cast<size_t>(channels);
    if (!frameBytes) {
        cleanup();
        return Err("Invalid audio frame format");
    }

    const size_t samplesPerFrame = std::max<size_t>(1, static_cast<size_t>(frequency) / ENVELOPE_HZ);
    std::vector<double> rms;
    std::vector<unsigned char> buffer(1 << 20), carry;
    size_t sampleCount = 0;
    double sumSquares = 0.0;
    while (true) {
        unsigned int readBytes = 0;
        auto readResult = sound->readData(buffer.data(), static_cast<unsigned int>(buffer.size()), &readBytes);
        if (!readBytes) break;
        carry.insert(carry.end(), buffer.begin(), buffer.begin() + readBytes);
        const size_t frames = carry.size() / frameBytes;
        for (size_t f = 0; f < frames; ++f) {
            double mixed = 0.0;
            for (int c = 0; c < channels; ++c) {
                const auto* p = carry.data() + f * frameBytes + static_cast<size_t>(c) * bytesPerSample;
                float v = 0.f;
                switch (format) {
                    case FMOD_SOUND_FORMAT_PCM8: v = (static_cast<int>(*p) - 128) / 128.f; break;
                    case FMOD_SOUND_FORMAT_PCM16: { int16_t s; std::memcpy(&s, p, 2); v = s / 32768.f; break; }
                    case FMOD_SOUND_FORMAT_PCM24: {
                        int32_t s = (static_cast<int32_t>(p[0]) << 8) |
                                    (static_cast<int32_t>(p[1]) << 16) |
                                    (static_cast<int32_t>(p[2]) << 24);
                        v = (s >> 8) / 8388608.f; break;
                    }
                    case FMOD_SOUND_FORMAT_PCM32: { int32_t s; std::memcpy(&s, p, 4); v = s / 2147483648.f; break; }
                    case FMOD_SOUND_FORMAT_PCMFLOAT: std::memcpy(&v, p, 4); break;
                    default: break;
                }
                mixed += v;
            }
            mixed /= channels;
            sumSquares += mixed * mixed;
            if (++sampleCount >= samplesPerFrame) {
                rms.push_back(std::sqrt(sumSquares / sampleCount));
                sampleCount = 0;
                sumSquares = 0.0;
            }
        }
        carry.erase(carry.begin(), carry.begin() + frames * frameBytes);
        if (readResult != FMOD_OK) break;
    }
    if (sampleCount) rms.push_back(std::sqrt(sumSquares / sampleCount));
    cleanup();
    if (rms.size() < ENVELOPE_HZ * 8)
        return Err("Song is too short for BPM analysis");

    const double mean = std::accumulate(rms.begin(), rms.end(), 0.0) / rms.size();
    std::vector<float> onset(rms.size(), 0.f);
    for (size_t i = 0; i < rms.size(); ++i) {
        const double prev = i ? rms[i - 1] : rms[i];
        const double next = i + 1 < rms.size() ? rms[i + 1] : rms[i];
        const double smooth = (prev + 2.0 * rms[i] + next) * 0.25;
        onset[i] = static_cast<float>(std::max(0.0, smooth - prev) +
                                      0.12 * std::max(0.0, smooth - mean));
    }
    return Ok(std::move(onset));
}

double correlation(std::vector<float> const& onset, int lag) {
    if (lag < 1 || static_cast<size_t>(lag) >= onset.size()) return 0.0;
    double cross = 0.0, a = 0.0, b = 0.0;
    const size_t n = onset.size() - static_cast<size_t>(lag);
    for (size_t i = 0; i < n; ++i) {
        const double x = onset[i], y = onset[i + static_cast<size_t>(lag)];
        cross += x * y; a += x * x; b += y * y;
    }
    return a > 1e-12 && b > 1e-12 ? cross / std::sqrt(a * b) : 0.0;
}
}

Result<BPMDetection> detectSongBPM(std::filesystem::path const& path) {
    auto decoded = decodeOnsets(path);
    if (!decoded) return Err(decoded.unwrapErr());
    auto const& onset = decoded.unwrap();

    const int minLag = static_cast<int>(std::floor(60.0 * ENVELOPE_HZ / MAX_BPM));
    const int maxLag = static_cast<int>(std::ceil(60.0 * ENVELOPE_HZ / MIN_BPM));
    std::vector<double> scores;
    scores.reserve(maxLag - minLag + 1);
    for (int lag = minLag; lag <= maxLag; ++lag) {
        scores.push_back(correlation(onset, lag)
            + 0.50 * correlation(onset, lag * 2)
            + 0.25 * correlation(onset, lag * 4)
            + 0.20 * correlation(onset, std::max(1, lag / 2)));
    }
    auto best = std::max_element(scores.begin(), scores.end());
    if (best == scores.end()) return Err("No beat pattern found");
    const int bestLag = minLag + static_cast<int>(best - scores.begin());
    const double bpm = 60.0 * ENVELOPE_HZ / bestLag;
    const double avg = std::accumulate(scores.begin(), scores.end(), 0.0) / scores.size();
    const double confidence = avg > 1e-9 ? *best / avg : 0.0;

    // Estimate the first beat phase from the strongest repeated onset position.
    std::vector<double> phaseEnergy(static_cast<size_t>(bestLag), 0.0);
    for (size_t i = 0; i < onset.size(); ++i)
        phaseEnergy[i % static_cast<size_t>(bestLag)] += onset[i];
    auto phase = std::max_element(phaseEnergy.begin(), phaseEnergy.end());
    const double firstBeat = static_cast<double>(phase - phaseEnergy.begin()) / ENVELOPE_HZ;
    return Ok(BPMDetection{bpm, firstBeat, confidence});
}
