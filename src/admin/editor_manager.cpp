#include "editor_manager.hpp"

namespace adm
{

void EditorManager::init(
    gui::GUIManager &guiManager,
    gfx::TextureCache &textureCache
)
{
    m_guiEditor.init(guiManager, textureCache);
    m_blockEditor.init(textureCache);
}

void EditorManager::render()
{
    if (!m_visible) return;

    ImGui::Begin("Editors", &m_visible, ImGuiWindowFlags_AlwaysAutoResize);
    
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 6));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(12, 8));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.5f, 0.7f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.6f, 0.8f, 0.9f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.7f, 0.9f, 1.0f));
    
    ImGui::Text("Select an editor:");
    ImGui::Separator();
    ImGui::Spacing();
    
    const float buttonWidth = ImGui::GetContentRegionAvail().x;
    
    if (ImGui::Button("GUI Editor", ImVec2(buttonWidth, 0))) 
       m_guiEditor.toggleVisible();
    
    if (ImGui::Button("Block Editor", ImVec2(buttonWidth, 0))) 
        m_blockEditor.toggleVisible();
    
    if (ImGui::Button("Sound Editor", ImVec2(buttonWidth, 0))) 
        m_soundEditor.toggleVisible();
    
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);

    m_guiEditor.render();
    m_blockEditor.render();
    m_soundEditor.render();

    ImGui::End();
}

} // namespace adm