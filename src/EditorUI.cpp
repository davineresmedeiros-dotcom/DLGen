#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>

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

        // Safe starter layout: continuous solid floor, no spikes or forced jumps.
        // This keeps the generated route traversable while the advanced pattern
        // and trigger logic is developed.
        constexpr float startX = 300.f;
        constexpr float blockWidth = 30.f;
        constexpr float floorY = 120.f;
        const int duration = getDurationSeconds();
        const int spacing = getOrbSpacing();

        // Conservative estimated horizontal distance at standard speed.
        // XL is five minutes; blocks are placed every 30 editor units.
        const float endX = startX + static_cast<float>(duration) * 300.f;
        for (float x = startX; x < endX; x += blockWidth) {
            editor->createObject(1, CCPoint{x, floorY}, false);
        }

        auto mod = Mod::get();
        if (mod->getSettingValue<std::string>("orbs") == "Active") {
            for (float x = startX + spacing; x < endX - 60.f; x += spacing) {
                // Yellow orb (object ID 35), placed above the continuous floor.
                editor->createObject(35, CCPoint{x, 180.f}, false);
            }
        }

        editor->m_editorUI->updateButtons();
        FLAlertLayer::create(
            "DLGen",
            "Safe starter layout generated. It uses a continuous floor and optional yellow orbs; no spikes or unavoidable obstacles were added.",
            "OK"
        )->show();
    }
};
