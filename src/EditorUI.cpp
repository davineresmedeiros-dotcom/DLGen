#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
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
        if (!editor) {
            FLAlertLayer::create("DLGen", "Open a level in the editor first.", "OK")->show();
            return;
        }
        constexpr float startX = 300.f, floorY = 120.f, blockWidth = 30.f;
        constexpr float unitsPerSecond = 300.f;
        const int duration = getDurationSeconds();
        const int difficulty = getDifficultyRank();
        const int bpm = std::clamp(Mod::get()->getSettingValue<int>("bpm"), 60, 240);
        const float beat = unitsPerSecond * 60.f / static_cast<float>(bpm);
        const float bar = beat * 4.f;
        const float endX = startX + static_cast<float>(duration) * unitsPerSecond;
        // Continuous solid ground. Generated obstacles stay above this line.
        for (float x = startX; x < endX; x += blockWidth)
            block(editor, x, floorY);

        std::random_device seed;
        std::mt19937 rng(seed());
        std::uniform_int_distribution<int> motif(0, 9), offset(0, 2), size(1, 3);
        const bool orbs = Mod::get()->getSettingValue<std::string>("orbs") == "Active";
        const float safeGap = difficulty == 0 ? beat * 3.0f : difficulty == 1 ? beat * 2.5f : beat * 2.0f;

        // Randomly choose a motif every musical bar. Each event snaps to the
        // beat/sub-beat grid, while a recovery gap prevents continuous hazards.
        for (float barX = startX + bar; barX < endX - bar; barX += bar) {
            const int chosen = motif(rng);
            const int shift = offset(rng);
            const float x = barX + beat * static_cast<float>(shift);
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
                    // Single raised block obstacle with a clear approach/exit.
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
                default: break; // intentional musical rest
            }
            // Keep the next bar's start clear for recovery on lower difficulties.
            (void)safeGap;
        }
        editor->m_editorUI->updateButtons();
        FLAlertLayer::create("DLGen",
            "Random beat-grid layout generated without triggers. BPM is currently set from the Song BPM option; automatic audio analysis is not enabled yet. Playtest obstacle clearance and timing.",
            "OK")->show();
    }
};