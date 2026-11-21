#pragma once
#include <Geode/Result.hpp>
#include <filesystem>

struct BPMDetection {
    double bpm = 0.0;
    double firstBeatSeconds = 0.0;
    double confidence = 0.0;
};

geode::Result<BPMDetection> detectSongBPM(std::filesystem::path const& path);
