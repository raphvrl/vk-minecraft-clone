#include "sound_editor.hpp"
#include "audio/sound_manager.hpp"

namespace adm
{

void SoundEditor::render()
{
    if (!m_visible) return;

    handleInput();

    ImGui::Begin("SOUND", &m_visible, ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit", "Ctrl+W")) {
                m_visible = false;
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Save", "Ctrl+S")) {
                sfx::SoundManager::get().save();
            }

            if (ImGui::MenuItem("Load", "Ctrl+O")) {
                sfx::SoundManager::get().load();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("New Sound")) {
                handleAddSound();
            }

            if (ImGui::MenuItem(
                "Duplicate",
                "Ctrl+D",
                false,
                m_selectedSound != nullptr
            )) {
                handleDuplicateSound();
            }

            if (ImGui::MenuItem(
                "Rename",
                "F2",
                false,
                m_selectedSound != nullptr
            )) {
                handleRenameSound();
            }

            if (ImGui::MenuItem(
                "Delete",
                "Del",
                false,
                m_selectedSound != nullptr
            )) {
                deleteSelectedSound();
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    ImGui::Separator();

    const f32 windowWidth = ImGui::GetWindowWidth();
    const f32 listWidth = windowWidth * 0.3f;
    const f32 propertiesWidth = windowWidth * 0.6f;

    ImGui::Columns(2, "##Columns", true);

    ImGui::SetColumnWidth(0, listWidth);
    renderSoundList();

    ImGui::NextColumn();

    renderSoundDetail();

    ImGui::End();

    renderNewSound();
    renderRenameSound();
}

void SoundEditor::renderSoundList()
{
    ImGui::InputTextWithHint(
        "Search",
        "",
        m_searchBuffer,
        sizeof(m_searchBuffer)
    );

    ImGui::BeginChild(
        "##SoundList",
        ImVec2(0, 0),
        true
    );

    json data = sfx::SoundManager::get().toJson();
    const json &categories = data["categories"];

    std::string searchStr = m_searchBuffer;
    std::transform(
        searchStr.begin(), 
        searchStr.end(),
        searchStr.begin(), 
        [](unsigned char c) { return std::tolower(c); }
    );

    renderJsonTree(categories, "", searchStr);

    ImGui::EndChild();
}

bool SoundEditor::renderJsonTree(
    const json &node,
    const std::string &path,
    const std::string &search
)
{
    bool anyChildVisible = false;
    
    for (auto it = node.begin(); it != node.end(); ++it) {
        const std::string& name = it.key();
        const json& content = it.value();

        std::string fullPath = path.empty() ? name : path + "." + name;

        bool isLeaf = content.contains("files");

        if (isLeaf) {
            if (!search.empty()) {
                std::string lowerPath = fullPath;
                std::transform(
                    lowerPath.begin(), 
                    lowerPath.end(),
                    lowerPath.begin(), 
                    [](unsigned char c) { return std::tolower(c); }
                );
                
                if (lowerPath.find(search) == std::string::npos) {
                    continue;
                }
            }

            ImGuiTreeNodeFlags leafFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 1.0f, 1.0f));
            ImGui::TreeNodeEx(name.c_str(), leafFlags);
            ImGui::PopStyleColor();
            
            if (ImGui::IsItemClicked()) {
                m_selectedPath = fullPath;
                m_selectedSound = sfx::SoundManager::get().getSound(fullPath);
            }
            
            anyChildVisible = true;
        } else {
            bool hasVisibleChildren = false;

            if (!search.empty()) {
                hasVisibleChildren = willAnyChildBeVisible(content, fullPath, search);
                if (!hasVisibleChildren) {
                    std::string lowerPath = fullPath;
                    std::transform(
                        lowerPath.begin(), 
                        lowerPath.end(),
                        lowerPath.begin(), 
                        [](unsigned char c) { return std::tolower(c); }
                    );
                    
                    if (lowerPath.find(search) == std::string::npos) {
                        continue;
                    }
                }
            }

            ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

            if (!search.empty() && hasVisibleChildren) {
                nodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
            }

            ImVec4 nodeColor = path.empty() ? 
                ImVec4(1.0f, 0.9f, 0.5f, 1.0f) :
                ImVec4(1.0f, 1.0f, 0.8f, 1.0f);
            
            ImGui::PushStyleColor(ImGuiCol_Text, nodeColor);
            bool isOpen = ImGui::TreeNodeEx(name.c_str(), nodeFlags);
            ImGui::PopStyleColor();
            
            if (isOpen) {
                bool childrenVisible = renderJsonTree(content, fullPath, search);
                
                if (!search.empty() && !childrenVisible) {
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "(No matching sounds)");
                }
                
                ImGui::TreePop();
            }
            
            anyChildVisible = anyChildVisible || hasVisibleChildren;
        }
    }
    
    return anyChildVisible;
}

bool SoundEditor::willAnyChildBeVisible(
    const json& node,
    const std::string& path,
    const std::string& search
)
{
    for (auto it = node.begin(); it != node.end(); ++it) {
        const std::string& name = it.key();
        const json& content = it.value();
        
        std::string fullPath = path.empty() ? name : path + "." + name;

        std::string lowerPath = fullPath;
        std::transform(
            lowerPath.begin(), 
            lowerPath.end(),
            lowerPath.begin(), 
            [](unsigned char c) { return std::tolower(c); }
        );
        
        if (lowerPath.find(search) != std::string::npos) {
            return true;
        }

        if (!content.contains("files")) {
            if (willAnyChildBeVisible(content, fullPath, search)) {
                return true;
            }
        }
    }
    
    return false;
}

void SoundEditor::renderSoundDetail()
{
    if (!m_selectedSound) {
        renderNoSoundSelected();
        return;
    }

    ImGui::BeginChild(
        "##SoundDetail",
        ImVec2(0, 0),
        true
    );

    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::Text("%s", m_selectedPath.c_str());
    ImGui::PopFont();

    ImGui::Separator();

    ImGui::Text("Properties:");
    ImGui::SliderFloat("Volume", &m_selectedSound->volume, 0.0f, 1.0f, "%.1f");
    ImGui::SliderFloat("Pitch", &m_selectedSound->pitch, 0.5f, 2.0f, "%.1f");

    f32 buttonWidth = ImGui::GetContentRegionAvail().x * 0.5f - 4.0f;
    
    if (ImGui::Button("Play", ImVec2(buttonWidth, 0))) {
        sfx::SoundManager::get().play(*m_selectedSound, false);
    }

    ImGui::SameLine();
    
    if (ImGui::Button("Stop", ImVec2(buttonWidth, 0))) {
        sfx::SoundManager::get().stop(*m_selectedSound);
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Sound Files:");

    f32 fileHeight = ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeightWithSpacing() * 4.0f;

    ImGui::BeginChild(
        "##SoundFiles",
        ImVec2(0, fileHeight),
        true
    );

    f32 deleteButtonX = ImGui::GetContentRegionAvail().x - 25.0f;

    for (size_t i = 0; i < m_selectedSound->sounds.size(); i++) {
        const auto &file = m_selectedSound->sounds[i];
        ImGui::PushID(static_cast<int>(i));
        
        ImGui::Bullet();
        ImGui::SameLine();
        ImGui::TextUnformatted(file.c_str());
        ImGui::SameLine();
        ImGui::SetCursorPosX(deleteButtonX);
        if (ImGui::Button("X##delete", ImVec2(20, 0))) {
            m_selectedSound->sounds.erase(m_selectedSound->sounds.begin() + i);
            ImGui::PopID();
            break;
        }
        ImGui::PopID();
    }

    ImGui::EndChild();

    static char inputBuffer[128] = "";
    ImGui::InputTextWithHint(
        "##AddFile",
        "Enter file path",
        inputBuffer,
        sizeof(inputBuffer)
    );

    ImGui::SameLine();
    
    if (ImGui::Button("Add File")) {
        if (inputBuffer[0] != '\0') {
            m_selectedSound->sounds.push_back(inputBuffer);
            sfx::SoundManager::get().updateSound(m_selectedPath);
        }
    }
    
    ImGui::EndChild();
}

void SoundEditor::renderNoSoundSelected()
{
    ImGui::BeginChild("##NoElementSelected", ImVec2(0, -1), true);
        
    ImVec2 region = ImGui::GetContentRegionAvail();
    f32 centerPosY = region.y * 0.4f;
    
    ImGui::Dummy(ImVec2(0, centerPosY));

    f32 textWidth = ImGui::CalcTextSize("No Sound Selected").x;
    f32 textPosX = (ImGui::GetWindowWidth() - textWidth) * 0.5f;
    
    ImGui::SetCursorPosX(textPosX);
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No Sound Selected");
    
    ImGui::Spacing();
    ImGui::Spacing();

    const std::string text = "Select a sound from the list to view its properties.";
    
    textWidth = ImGui::CalcTextSize(
        text.c_str()
    ).x;
    textPosX = (ImGui::GetWindowWidth() - textWidth) * 0.5f;
    
    ImGui::SetCursorPosX(textPosX);
    ImGui::TextColored(
        ImVec4(0.5f, 0.5f, 0.5f, 1.0f), 
        text.c_str()
    );
    
    ImGui::EndChild();
}

void SoundEditor::renderNewSound()
{
    if (m_showAddSoundPopup) {
        ImGui::OpenPopup("Add New Sound");
    }

    ImGui::SetNextWindowSize(ImVec2(400, 250), ImGuiCond_FirstUseEver);

    if (ImGui::BeginPopupModal("Add New Sound", &m_showAddSoundPopup, ImGuiWindowFlags_AlwaysAutoResize)) {
        static char pathBuffer[128] = "";
        static char nameBuffer[64] = "";
        static char categoryBuffer[64] = "";
        static f32 volume = 1.0f;
        static f32 pitch = 1.0f;

        ImGui::Text("Enter sound information:");
        ImGui::Separator();

        ImGui::InputTextWithHint(
            "Category",
            "e.g. ambient.cave",
            categoryBuffer,
            sizeof(categoryBuffer)
        );
        ImGui::InputTextWithHint(
            "Name", 
            "Sound identifier", 
            nameBuffer, 
            sizeof(nameBuffer)
        );
        ImGui::InputTextWithHint(
            "File Path", 
            "Optional: e.g. sounds/ambient/cave1.ogg", 
            pathBuffer, 
            sizeof(pathBuffer)
        );
        
        ImGui::Spacing();
        ImGui::SliderFloat("Volume", &volume, 0.0f, 1.0f);
        ImGui::SliderFloat("Pitch", &pitch, 0.5f, 2.0f);
        
        ImGui::Separator();
        
        if (ImGui::Button("Create", ImVec2(120, 0))) {
            if (categoryBuffer[0] != '\0' && nameBuffer[0] != '\0') {
                std::string fullPath = std::string(categoryBuffer) + "." + nameBuffer;
                
                sfx::Sound newSound;
                newSound.volume = volume;
                newSound.pitch = pitch;

                if (pathBuffer[0] != '\0') {
                    newSound.sounds.push_back(pathBuffer);
                }
                
                sfx::SoundManager::get().addSound(fullPath, newSound);

                pathBuffer[0] = '\0';
                nameBuffer[0] = '\0';
                categoryBuffer[0] = '\0';
                volume = 1.0f;
                pitch = 1.0f;
                
                m_showAddSoundPopup = false;
                ImGui::CloseCurrentPopup();
            }
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            m_showAddSoundPopup = false;
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
}

void SoundEditor::renderRenameSound()
{
    if (m_showRenameSoundPopup) {
        ImGui::OpenPopup("Rename Sound");
    }

    ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_FirstUseEver);

    if (ImGui::BeginPopupModal(
        "Rename Sound",
        &m_showRenameSoundPopup,
        ImGuiWindowFlags_AlwaysAutoResize
    )) {
        static char categoryBuffer[128] = "";
        static char nameBuffer[64] = "";

        if (ImGui::IsWindowAppearing() && m_selectedPath.length() > 0) {
            size_t lastDotPos = m_selectedPath.find_last_of('.');
            if (lastDotPos != std::string::npos) {
                std::string category = m_selectedPath.substr(0, lastDotPos);
                std::string name = m_selectedPath.substr(lastDotPos + 1);
                
                strncpy(categoryBuffer, category.c_str(), sizeof(categoryBuffer) - 1);
                categoryBuffer[sizeof(categoryBuffer) - 1] = '\0';
                
                strncpy(nameBuffer, name.c_str(), sizeof(nameBuffer) - 1);
                nameBuffer[sizeof(nameBuffer) - 1] = '\0';
            }
        }

        ImGui::Text("Rename sound:");
        ImGui::Separator();

        ImGui::InputTextWithHint(
            "Category",
            "ambient.cave",
            categoryBuffer,
            sizeof(categoryBuffer)
        );
        
        ImGui::InputTextWithHint(
            "Sound Name",
            "drip",
            nameBuffer,
            sizeof(nameBuffer)
        );

        ImGui::Separator();

        if (ImGui::Button("Rename", ImVec2(120, 0))) {
            if (categoryBuffer[0] != '\0' && nameBuffer[0] != '\0') {
                std::string newPath = std::string(categoryBuffer) + "." + nameBuffer;
                
                sfx::SoundManager::get().renameSound(m_selectedPath, newPath);
                
                m_selectedPath = newPath;
                m_selectedSound = sfx::SoundManager::get().getSound(newPath);
                
                categoryBuffer[0] = '\0';
                nameBuffer[0] = '\0';
                
                m_showRenameSoundPopup = false;
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            m_showRenameSoundPopup = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void SoundEditor::handleInput()
{
    bool ctrlPressed = ImGui::GetIO().KeyCtrl;

    if (ctrlPressed) {
        if (ImGui::IsKeyPressed(ImGuiKey_W)) {
            m_visible = false;
        }

        if (ImGui::IsKeyPressed(ImGuiKey_S)) {
            sfx::SoundManager::get().save();
        }

        if (ImGui::IsKeyPressed(ImGuiKey_O)) {
            sfx::SoundManager::get().load();
        }

        if (ImGui::IsKeyPressed(ImGuiKey_N)) {
            handleAddSound();
        }

        if (ImGui::IsKeyPressed(ImGuiKey_D) && m_selectedSound) {
            handleDuplicateSound();
        }
    }

    if (ImGui::IsKeyPressed(ImGuiKey_F2) && m_selectedSound) {
        handleRenameSound();
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Delete) && m_selectedSound) {
        deleteSelectedSound();
    }
}

void SoundEditor::handleAddSound()
{
    m_showAddSoundPopup = true;
}

void SoundEditor::handleDuplicateSound()
{
    if (m_selectedSound && !m_selectedPath.empty()) {
        sfx::Sound duplicatedSound = *m_selectedSound;

        std::string basePath = m_selectedPath;
        std::string newPath = basePath + "_copy";
        
        int copyNumber = 1;
        while (sfx::SoundManager::get().getSound(newPath) != nullptr) {
            copyNumber++;
            newPath = basePath + "_copy" + std::to_string(copyNumber);
        }

        sfx::SoundManager::get().addSound(newPath, duplicatedSound);

        m_selectedPath = newPath;
        m_selectedSound = sfx::SoundManager::get().getSound(newPath);
    }
}

void SoundEditor::handleRenameSound()
{
    m_showRenameSoundPopup = true;
}

void SoundEditor::deleteSelectedSound()
{
    if (m_selectedSound) {
        sfx::SoundManager::get().removeSound(m_selectedPath);
        m_selectedSound = nullptr;
        m_selectedPath.clear();
    }
}

} // namespace adm