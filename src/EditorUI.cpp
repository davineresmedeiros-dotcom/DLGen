#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include "BPMDetector.hpp"
#include <random>
#include <algorithm>

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

        // Resolve and analyze the actual song file selected by this level.
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

        constexpr float startX = 300.f, floorY = 120.f, blockWidth = 30.f;
        constexpr float unitsPerSecond = 300.f;
        const int duration = getDurationSeconds();
        const int difficulty = getDifficultyRank();
        const float beat = unitsPerSecond * 60.f / bpm;
        const float bar = beat * 4.f;
        const float songPhaseX = startX + static_cast<float>(timing.firstBeatSeconds) * unitsPerSecond;
        const float endX = startX + static_cast<float>(duration) * unitsPerSecond;

        for (float x = startX; x < endX; x += blockWidth)
            block(editor, x, floorY);

        std::random_device seed;
        std::mt19937 rng(seed());
        std::uniform_int_distribution<int> motif(0, 9), offset(0, 2), size(1, 3);
        const bool orbs = Mod::get()->getSettingValue<std::string>("orbs") == "Active";

        // Phrases start on the detected beat phase, not a manually entered BPM.
        for (float barX = songPhaseX + bar; barX < endX - bar; barX += bar) {
            const int chosen = motif(rng);
            const float x = barX + beat * static_cast<float>(offset(rng));
            const int count = size(rng);
            switch (chosen) {
                case 0: case 1: spikes(editor, x, floorY, 1); break;
                case 2: spikes(editor, x, floorY, 2); break;
                case 3: spikes(editor, x, floorY, count); break;
                case 4:
                    spikes(editor, x, floorY, 1);
                    if (orbs) orb(editor, x + beat, floorY + 65.f);
                    break;
                case 5:
                    spikes(editor, x, floorY, 1);
                    spikes(editor, x + beat * 2.f, floorY, 1);
                    break;
                case 6:
                    block(editor, x + beat, floorY + 30.f);
                    if (orbs) orb(editor, x + beat, floorY + 95.f);
                    break;
                case 7:
                    spikes(editor, x + beat, floorY, 2);
                    if (orbs) orb(editor, x + beat * 2.f, floorY + 65.f);
                    break;
                case 8:
                    block(editor, x, floorY + 30.f);
                    block(editor, x + beat, floorY + 30.f);
                    break;
                default: break;
            }
        }
        editor->m_editorUI->updateButtons();
        FLAlertLayer::create("DLGen",
            fmt::format("Audio BPM detected: {:.1f}\nBeat phase analyzed automatically. Random spikes, blocks and orbs generated without triggers.", bpm),
            "OK")->show();
    }
};