#pragma once

#include "imgui_manager.hpp"
#include "gui_editor.hpp"
#include "block_editor.hpp"
#include "sound_editor.hpp"

namespace adm
{

class EditorManager
{

public:
    EditorManager() = default;
    ~EditorManager() = default;

    void init(
        gui::GUIManager &guiManager,
        gfx::TextureCache &textureCache
    );

    void render();

    bool isVisible() const { return m_visible; }
    void setVisible(bool visible) { m_visible = visible; }
    void toggleVisible() { m_visible = !m_visible; }

private:
    bool m_visible = false;

    GUIEditor m_guiEditor;
    BlockEditor m_blockEditor;
    SoundEditor m_soundEditor;

};

} // namespace adm