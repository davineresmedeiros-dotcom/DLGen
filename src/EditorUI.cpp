#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>

using namespace geode::prelude;

class $modify(DLGenEditorUI, EditorUI) {
    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer))
            return false;

        // Add a DLGen shortcut to the editor's existing playback menu.
        auto menu = this->getChildByID("playback-menu");
        if (!menu) {
            log::warn("DLGen: editor playback menu was not found; settings remain available from the Geode mod page.");
            return true;
        }

        auto sprite = ButtonSprite::create("DLGen");
        sprite->setScale(0.8f);

        auto button = CCMenuItemSpriteExtra::create(
            sprite,
            this,
            menu_selector(DLGenEditorUI::onDLGenSettings)
        );
        button->setID("dlgen-settings-button");

        menu->addChild(button);
        m_uiItems->addObject(button);
        menu->updateLayout();

        return true;
    }

    void onDLGenSettings(CCObject*) {
        geode::openSettingsPopup(Mod::get(), false);
    }
};
