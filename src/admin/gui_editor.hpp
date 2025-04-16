#pragma once

#include "imgui_manager.hpp"
#include "gui/gui_manager.hpp"

namespace adm
{

class GUIEditor
{

public:
    GUIEditor() = default;
    ~GUIEditor() = default;

    void init(
        ImGuiManager &imguiManager,
        gui::GUIManager &guiManager,
        gfx::TextureCache &textureCache
    );

    void render();

    bool isVisible() const { return m_visible; }
    void setVisible(bool visible) { m_visible = visible; }
    void toggleVisible() { m_visible = !m_visible; }

private:
    ImGuiManager *m_imguiManager = nullptr;
    gui::GUIManager *m_guiManager = nullptr;
    gfx::TextureCache *m_textureCache = nullptr;

    bool m_visible = false;
    
    std::string m_selectedElementID = "";
    std::shared_ptr<gui::GUIElement> m_selectedElement = nullptr;

    void renderElementList();
    void renderElementProperties();
    void renderAnchorSelector();
    void renderTextureSelector();
    void renderNoElementSelected();

    void createNewElement();
    void deleteSelectedElement();
    void duplicateSelectedElement();


};

} // namespace adm