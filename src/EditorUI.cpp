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

int getOrbSpacing() {
    auto difficulty = Mod::get()->getSettingValue<std::string>("difficulty");
    if (difficulty == "Easy") return 900;
    if (difficulty == "Hard") return 450;
    if (difficulty == "Insane") return 360;
    if (difficulty == "Demon") return 300;
    return 600; // Normal
}

GameObject* addGroupedObject(LevelEditorLayer* editor, int objectID, CCPoint position, int groupID) {
    auto object = editor->createObject(objectID, position, false);
    if (object) object->addGroup(groupID);
    return object;
}

void addToggleTrigger(LevelEditorLayer* editor, float x, float y, int groupID, bool activate) {
    auto object = editor->createObject(1049, CCPoint{x, y}, false);
    if (!object) return;
    auto trigger = static_cast<EffectGameObject*>(object);
    trigger->m_targetGroupID = groupID;
    trigger->m_activateGroup = activate;
}

int getDifficultyRank() {
    auto difficulty = Mod::get()->getSettingValue<std::string>("difficulty");
    if (difficulty == "Easy") return 0;
    if (difficulty == "Normal") return 1;
    if (difficulty == "Hard") return 2;
    if (difficulty == "Insane") return 3;
    if (difficulty == "Demon") return 4;
    return 1;
}
}

class $modify(DLGenEditorUI, EditorUI) {
    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer))
            return false;

        auto menu = this->getChildByID("playback-menu");
        if (!menu) {
            log::warn("DLGen: playback menu not found.");
            return true;
        }

        auto settingsSprite = ButtonSprite::create("DLGen");
        settingsSprite->setScale(0.8f);
        auto settingsButton = CCMenuItemSpriteExtra::create(
            settingsSprite, this, menu_selector(DLGenEditorUI::onDLGenSettings));
        settingsButton->setID("dlgen-settings-button");
        menu->addChild(settingsButton);
        m_uiItems->addObject(settingsButton);

        auto generateSprite = ButtonSprite::create("Generate");
        generateSprite->setScale(0.8f);
        auto generateButton = CCMenuItemSpriteExtra::create(
            generateSprite, this, menu_selector(DLGenEditorUI::onDLGenGenerate));
        generateButton->setID("dlgen-generate-button");
        menu->addChild(generateButton);
        m_uiItems->addObject(generateButton);

        menu->updateLayout();
        return true;
    }

    void onDLGenSettings(CCObject*) {
        geode::openSettingsPopup(Mod::get(), false);
    }

    void onDLGenGenerate(CCObject*) {
        auto editor = m_editorLayer;
        if (!editor) {
            FLAlertLayer::create("DLGen", "Open a level in the editor first.", "OK")->show();
            return;
        }

        // Organized procedural layout: each phrase has a safe lead-in,
        // a small randomized obstacle pattern, and a recovery gap.
        // We deliberately avoid triple spikes and tightly chained jumps.
        constexpr float startX = 300.f;
        constexpr float blockWidth = 30.f;
        constexpr float floorY = 120.f;
        const int duration = getDurationSeconds();
        const int spacing = getOrbSpacing();
        const int difficulty = getDifficultyRank();

        const float endX = startX + static_cast<float>(duration) * 300.f;
        for (float x = startX; x < endX; x += blockWidth) {
            editor->createObject(1, CCPoint{x, floorY}, false);
        }

        std::random_device seed;
        std::mt19937 rng(seed());
        std::uniform_int_distribution<int> patternRoll(0, 99);
        std::uniform_int_distribution<int> offsetRoll(0, 120);

        // Repeating phrases keep the level readable; randomness only chooses
        // among patterns appropriate for the selected difficulty.
        constexpr float phraseLength = 960.f;
        int phraseIndex = 0;
        for (float phrase = startX + 360.f; phrase < endX - 360.f; phrase += phraseLength, ++phraseIndex) {
            const int groupID = phraseIndex + 1;
            const int roll = patternRoll(rng);
            if (Mod::get()->getSettingValue<std::string>("triggers") == "Yes") {
                // Turn this phrase group off, then back on before its obstacles.
                addToggleTrigger(editor, phrase - 240.f, floorY, groupID, false);
                addToggleTrigger(editor, phrase - 120.f, floorY, groupID, true);
            }
            const float patternX = phrase + static_cast<float>(offsetRoll(rng));

            if (difficulty == 0) {
                // Easy: usually one isolated spike, with frequent empty phrases.
                if (roll < 65)
                    addGroupedObject(editor, 8, CCPoint{patternX, floorY}, groupID);
            }
            else if (difficulty == 1) {
                // Normal: isolated spike or two singles with a wide gap.
                if (roll < 45) {
                    editor->createObject(8, CCPoint{patternX, floorY}, false);
                } else if (roll < 75) {
                    editor->createObject(8, CCPoint{patternX, floorY}, false);
                    addGroupedObject(editor, 8, CCPoint{patternX + 390.f, floorY}, groupID);
                }
            }
            else if (difficulty == 2) {
                // Hard: occasional double spike, never triple.
                if (roll < 35) {
                    editor->createObject(8, CCPoint{patternX, floorY}, false);
                } else if (roll < 65) {
                    editor->createObject(8, CCPoint{patternX, floorY}, false);
                    addGroupedObject(editor, 8, CCPoint{patternX + 30.f, floorY}, groupID);
                } else if (roll < 85) {
                    editor->createObject(8, CCPoint{patternX, floorY}, false);
                    addGroupedObject(editor, 8, CCPoint{patternX + 420.f, floorY}, groupID);
                }
            }
            else {
                // Insane/Demon: varied singles and doubles with recovery space.
                if (roll < 25) {
                    editor->createObject(8, CCPoint{patternX, floorY}, false);
                } else if (roll < 60) {
                    editor->createObject(8, CCPoint{patternX, floorY}, false);
                    editor->createObject(8, CCPoint{patternX + 30.f, floorY}, false);
                } else if (roll < 82) {
                    editor->createObject(8, CCPoint{patternX, floorY}, false);
                    addGroupedObject(editor, 8, CCPoint{patternX + 480.f, floorY}, groupID);
                }
            }
        }

        auto mod = Mod::get();
        if (mod->getSettingValue<std::string>("orbs") == "Active") {
            const int phraseCount = std::max(1, phraseIndex);
            for (float x = startX + spacing; x < endX - 60.f; x += spacing) {
                // Yellow orb (object ID 35), placed above the continuous floor.
                const int groupID = std::clamp(
                    1 + static_cast<int>((x - (startX + 360.f)) / phraseLength),
                    1, phraseCount
                );
                addGroupedObject(editor, 35, CCPoint{x, 180.f}, groupID);
            }
        }

        editor->m_editorUI->updateButtons();
        FLAlertLayer::create(
            "DLGen",
            "Organized randomized layout generated: safe lead-ins, short obstacle patterns, and recovery gaps. Please playtest the result; gameplay timing is still being tuned.",
            "OK"
        )->show();
    }
};
