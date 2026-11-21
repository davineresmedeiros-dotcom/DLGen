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
GameObject* addObject(LevelEditorLayer* editor, int objectID, float x, float y) {
    return editor->createObject(objectID, CCPoint{x, y}, false);
}
void addSpikeRun(LevelEditorLayer* editor, float x, float y, int count) {
    for (int i = 0; i < count; ++i)
        addObject(editor, 8, x + 30.f * static_cast<float>(i), y);
}
void addOrb(LevelEditorLayer* editor, float x, float y) {
    addObject(editor, 35, x, y);
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
        if (!editor) {
            FLAlertLayer::create("DLGen", "Open a level in the editor first.", "OK")->show();
            return;
        }
        constexpr float startX = 300.f, blockWidth = 30.f, floorY = 120.f;
        constexpr float unitsPerSecond = 300.f;
        const int duration = getDurationSeconds();
        const int difficulty = getDifficultyRank();
        int bpm = std::clamp(Mod::get()->getSettingValue<int>("bpm"), 60, 240);
        const float beat = unitsPerSecond * 60.f / static_cast<float>(bpm);
        const float bar = beat * 4.f;
        const float endX = startX + static_cast<float>(duration) * unitsPerSecond;
        for (float x = startX; x < endX; x += blockWidth)
            addObject(editor, 1, x, floorY);

        std::random_device seed;
        std::mt19937 rng(seed());
        std::uniform_int_distribution<int> patternRoll(0, 7);
        std::uniform_int_distribution<int> offsetRoll(0, 3);
        std::uniform_int_distribution<int> variantRoll(0, 2);
        const bool orbs = Mod::get()->getSettingValue<std::string>("orbs") == "Active";

        // A random phrase is selected every four beats, with its events quantized
        // to the BPM grid. Higher difficulties select denser phrase families.
        int phraseIndex = 0;
        for (float phraseX = startX + bar; phraseX < endX - bar; phraseX += bar, ++phraseIndex) {
            const int pattern = patternRoll(rng);
            const int variant = variantRoll(rng);
            const float x = phraseX + beat * static_cast<float>(offsetRoll(rng));
            if (difficulty == 0) {
                switch (pattern) {
                    case 0: case 1: addSpikeRun(editor, x, floorY, 1); break;
                    case 2: addSpikeRun(editor, x, floorY, 2); break;
                    case 3: addSpikeRun(editor, x + beat, floorY, 1); break;
                    case 4: if (orbs) addOrb(editor, x + beat, floorY + 60.f); break;
                    default: break;
                }
            } else if (difficulty == 1) {
                switch (pattern) {
                    case 0: case 1: addSpikeRun(editor, x, floorY, 1); break;
                    case 2: addSpikeRun(editor, x, floorY, 2); break;
                    case 3: addSpikeRun(editor, x + beat, floorY, 2); break;
                    case 4: addSpikeRun(editor, x, floorY, 1); addSpikeRun(editor, x + beat * 2.f, floorY, 1); break;
                    case 5: if (orbs) addOrb(editor, x + beat, floorY + 60.f); break;
                    default: break;
                }
            } else {
                switch (pattern) {
                    case 0: addSpikeRun(editor, x, floorY, 2); break;
                    case 1: addSpikeRun(editor, x, floorY, 1 + variant); break;
                    case 2: addSpikeRun(editor, x, floorY, 1); addSpikeRun(editor, x + beat * 2.f, floorY, 2); break;
                    case 3: addSpikeRun(editor, x + beat, floorY, 2); break;
                    case 4: addSpikeRun(editor, x, floorY, 1); addSpikeRun(editor, x + beat, floorY, 1); addSpikeRun(editor, x + beat * 2.f, floorY, 1); break;
                    case 5: if (orbs) addOrb(editor, x + beat, floorY + 60.f); break;
                    case 6: addSpikeRun(editor, x + beat, floorY, 1); addSpikeRun(editor, x + beat * 3.f, floorY, 1); break;
                    default: break;
                }
            }
            if (orbs && difficulty >= 2 && pattern == 7 && variant == 0)
                addOrb(editor, phraseX + beat * 3.f, floorY + 75.f);
        }
        editor->m_editorUI->updateButtons();
        FLAlertLayer::create("DLGen",
            "Random beat-grid layout generated. Set Song BPM to match the music for rhythm alignment. Patterns and obstacle density vary by difficulty; playtest the jumps and spacing.",
            "OK")->show();
    }
};