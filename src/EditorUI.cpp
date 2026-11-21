#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include "BPMDetector.hpp"
#include <random>
#include <algorithm>
#include <cmath>

using namespace geode::prelude;

namespace {
int getDurationSeconds() {
    auto time = Mod::get()->getSettingValue<std::string>("time");
    if (time == "Long") return 120;
    if (time == "XL") return 300;
    return 60;
}
int getDifficultyRank() {
    auto difficulty = Mod::get()->getSettingValue<std::string>("difficulty");
    if (difficulty == "Easy") return 0;
    if (difficulty == "Hard") return 2;
    if (difficulty == "Insane") return 3;
    if (difficulty == "Demon") return 4;
    return 1;
}
GameObject* put(LevelEditorLayer* editor, int id, float x, float y) {
    return editor->createObject(id, CCPoint{x, y}, false);
}
void spikes(LevelEditorLayer* editor, float x, float y, int count) {
    for (int i = 0; i < count; ++i)
        put(editor, 8, x + 30.f * static_cast<float>(i), y);
}
void orb(LevelEditorLayer* editor, float x, float y) { put(editor, 35, x, y); }
void block(LevelEditorLayer* editor, float x, float y) { put(editor, 1, x, y); }
void platform(LevelEditorLayer* editor, float x, float y, int count) {
    for (int i = 0; i < count; ++i)
        block(editor, x + 30.f * static_cast<float>(i), y);
}
}

class $modify(DLGenEditorUI, EditorUI) {
    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer)) return false;
        auto menu = this->getChildByID("playback-menu");
        if (!menu) {
            log::warn("DLGen: playback menu not found.");
            return true;
        }
        auto settingsSprite = ButtonSprite::create("DLGen");
        settingsSprite->setScale(0.8f);
        auto settingsButton = CCMenuItemSpriteExtra::create(settingsSprite, this, menu_selector(DLGenEditorUI::onDLGenSettings));
        settingsButton->setID("dlgen-settings-button");
        menu->addChild(settingsButton);
        m_uiItems->addObject(settingsButton);
        auto generateSprite = ButtonSprite::create("Generate");
        generateSprite->setScale(0.8f);
        auto generateButton = CCMenuItemSpriteExtra::create(generateSprite, this, menu_selector(DLGenEditorUI::onDLGenGenerate));
        generateButton->setID("dlgen-generate-button");
        menu->addChild(generateButton);
        m_uiItems->addObject(generateButton);
        menu->updateLayout();
        return true;
    }
    void onDLGenSettings(CCObject*) { geode::openSettingsPopup(Mod::get(), false); }
    void onDLGenGenerate(CCObject*) {
        auto editor = m_editorLayer;
        if (!editor || !editor->m_level) {
            FLAlertLayer::create("DLGen", "Open a level with a selected song in the editor first.", "OK")->show();
            return;
        }
        auto audioFile = editor->m_level->getAudioFileName();
        if (audioFile.empty()) {
            FLAlertLayer::create("DLGen", "Could not find the selected song file. Select/download a song and try again.", "OK")->show();
            return;
        }
        auto resolvedFile = CCFileUtils::get()->fullPathForFilename(audioFile.c_str(), false);
        std::filesystem::path audioPath(resolvedFile.empty() ? audioFile.c_str() : resolvedFile.c_str());
        auto detected = detectSongBPM(audioPath);
        if (!detected) {
            FLAlertLayer::create("DLGen", fmt::format("Could not analyze this song's audio.\n{}", detected.unwrapErr()), "OK")->show();
            return;
        }
        const auto timing = detected.unwrap();
        const float bpm = static_cast<float>(timing.bpm);
        if (bpm < 70.f || bpm > 220.f || timing.confidence < 1.03) {
            FLAlertLayer::create("DLGen", fmt::format("BPM detection was uncertain ({:.1f} BPM). Try a clearer song file.", bpm), "OK")->show();
            return;
        }

        constexpr float startX = 300.f, groundY = 120.f;
        constexpr float unitsPerSecond = 300.f;
        const int duration = getDurationSeconds();
        const int difficulty = getDifficultyRank();
        const float beat = unitsPerSecond * 60.f / bpm;
        const float bar = beat * 4.f;
        const float phraseWidth = bar * 2.f;
        const float phaseX = startX + static_cast<float>(timing.firstBeatSeconds) * unitsPerSecond;
        const float endX = startX + static_cast<float>(duration) * unitsPerSecond;

        std::random_device seed;
        std::mt19937 rng(seed());
        std::uniform_int_distribution<int> motif(0, 11);
        std::uniform_int_distribution<int> smallCount(1, 2);
        std::uniform_int_distribution<int> platformCount(3, 6);
        std::uniform_int_distribution<int> height(1, 3);
        std::uniform_int_distribution<int> rest(0, 3);
        const bool orbs = Mod::get()->getSettingValue<std::string>("orbs") == "Active";

        // Leave the game's native ground alone: don't stamp thousands of blocks
        // across the whole level. Build distinct, beat-quantized phrases instead.
        int phraseNumber = 0;
        for (float phraseX = phaseX + phraseWidth; phraseX < endX - phraseWidth;
             phraseX += phraseWidth, ++phraseNumber) {
            const int style = motif(rng);
            const float x = phraseX + beat * static_cast<float>(rest(rng));
            const float elevatedY = groundY + 30.f * static_cast<float>(height(rng));
            const int n = platformCount(rng);

            switch (style) {
                // Ground-based rhythms with different spacing and recovery.
                case 0: spikes(editor, x, groundY, 1); break;
                case 1: spikes(editor, x, groundY, 2); break;
                case 2:
                    spikes(editor, x, groundY, 1);
                    spikes(editor, x + beat * 2.f, groundY, 1);
                    break;
                case 3:
                    spikes(editor, x, groundY, smallCount(rng));
                    if (orbs) orb(editor, x + beat, groundY + 65.f);
                    break;
                // Short raised platforms with a beat-matched approach.
                case 4:
                    platform(editor, x + beat, elevatedY, n);
                    if (orbs) {
                        orb(editor, x + beat * 1.5f, elevatedY + 55.f);
                        orb(editor, x + beat * 2.5f, elevatedY + 55.f);
                    }
                    break;
                case 5:
                    platform(editor, x, groundY + 30.f, 3);
                    platform(editor, x + beat * 2.f, groundY + 60.f, 3);
                    break;
                // Staircase motifs: ascending or descending block groups.
                case 6:
                    for (int i = 0; i < 3; ++i)
                        platform(editor, x + beat * static_cast<float>(i),
                                 groundY + 30.f * static_cast<float>(i + 1), 2);
                    break;
                case 7:
                    for (int i = 0; i < 3; ++i)
                        platform(editor, x + beat * static_cast<float>(i),
                                 groundY + 60.f - 30.f * static_cast<float>(i), 2);
                    break;
                // Floating block formations and orb arcs.
                case 8:
                    platform(editor, x + beat, elevatedY, 2);
                    platform(editor, x + beat * 2.f, elevatedY + 30.f, 2);
                    platform(editor, x + beat * 3.f, elevatedY, 2);
                    if (orbs) orb(editor, x + beat * 2.f, elevatedY + 90.f);
                    break;
                case 9:
                    block(editor, x + beat, groundY + 30.f);
                    block(editor, x + beat * 2.f, groundY + 60.f);
                    block(editor, x + beat * 3.f, groundY + 30.f);
                    if (orbs) orb(editor, x + beat * 2.f, groundY + 115.f);
                    break;
                // A mixed phrase ending in a safe rest.
                case 10:
                    spikes(editor, x, groundY, 1);
                    platform(editor, x + beat * 2.f, groundY + 30.f, 3);
                    if (orbs) orb(editor, x + beat * 2.5f, groundY + 100.f);
                    break;
                default:
                    // Intentional musical rest for contrast.
                    break;
            }
        }

        editor->m_editorUI->updateButtons();
        FLAlertLayer::create("DLGen",
            fmt::format("BPM detected: {:.1f}\nGenerated varied beat-synced phrases with platforms, stairs, blocks, spikes and optional orbs. No artificial continuous floor or triggers.", bpm),
            "OK")->show();
    }
};