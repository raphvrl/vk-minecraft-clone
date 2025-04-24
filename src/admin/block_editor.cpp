#include "block_editor.hpp"
#include "audio/sound_manager.hpp"

namespace adm
{

void BlockEditor::init(gfx::TextureCache &textureCache)
{
    m_textureCache = &textureCache;
}

void BlockEditor::render() {
    if (!m_visible) return;

    handleInput();

    ImGui::Begin("BLOCK", &m_visible, ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Block", "Ctrl+N")) {
                createNewBlock();
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Save", "Ctrl+S")) {
                wld::BlockRegistry::get().save();
            }

            if (ImGui::MenuItem("Load", "Ctrl+O")) {
                wld::BlockRegistry::get().load();
            }
        
            ImGui::Separator();

            if (ImGui::MenuItem("Exit", "Ctrl+W")) {
                m_visible = false;
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem(
                "Duplicate",
                "Ctrl+D",
                m_selectedBlockName != ""
            )) {
                duplicateSelectedBlock();
            }

            if (ImGui::MenuItem(
                "Delete",
                "Del",
                m_selectedBlockName != ""
            )) {
                deleteSelectedBlock();
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
    renderBlockList();

    ImGui::NextColumn();

    if (ImGui::BeginTabBar("PropertiesTabs")) {
        if (ImGui::BeginTabItem("Properties")) {
            renderBlockProperties();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Textures")) {
            renderTextureSelector();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Sounds")) {
            renderSoundSelector();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::Columns(1);

    ImGui::End();
}

void BlockEditor::renderBlockList()
{
    static char searchBuffer[128] = "";
    ImGui::InputTextWithHint(
        "Search",
        "",
        searchBuffer,
        sizeof(searchBuffer)
    );

    std::string searchStr = searchBuffer;
    std::transform(
        searchStr.begin(),
        searchStr.end(),
        searchStr.begin(),
        [](unsigned char c) { return std::tolower(c); }
    );

    const auto& blocks = wld::BlockRegistry::get().getBlocks();
    if (blocks.empty()) {
        ImGui::TextColored(
            ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
            "No elements found."
        );
        return;
    };

    ImGui::BeginChild(
        "##BlockList",
        ImVec2(0, -ImGui::GetFrameHeightWithSpacing()),
        true
    );

    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.4f, 0.8f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.5f, 0.9f, 0.7f));

    ImVec4 selectedTextColor(1.0f, 1.0f, 0.0f, 1.0f);

    for (const auto& [blockId, block] : blocks) {
        if (blockId == 0 || block.name.empty())
            continue;

        std::string label = block.name;

        std::string lowerLabel = label;
        std::transform(
            lowerLabel.begin(), 
            lowerLabel.end(), 
            lowerLabel.begin(),
            [](unsigned char c) { return std::tolower(c); }
        );

        if (!searchStr.empty() && lowerLabel.find(searchStr) == std::string::npos) {
            continue;
        }

        std::string displayText = "[" + std::to_string(blockId) + "] " + label;

        bool isSelected = (m_selectedBlockID == blockId);
        if (isSelected) {
            ImGui::PushStyleColor(ImGuiCol_Text, selectedTextColor);
        }

        if (ImGui::Selectable(displayText.c_str(), isSelected)) {
            m_selectedBlockName = block.name;
            m_selectedBlock = &wld::BlockRegistry::get().getBlock(blockId);
            m_selectedBlockID = blockId;
        }

        if (isSelected) {
            ImGui::PopStyleColor(1);
        }
    }

    ImGui::PopStyleColor(2);

    ImGui::EndChild();

    if (ImGui::Button("+ Add Block", ImVec2(-1, 0))) {
        createNewBlock();
    }
}

void BlockEditor::renderBlockProperties()
{
    if (m_selectedBlockName.empty()) {
        renderNoBlockSelected();
        return;
    }

    ImGui::BeginChild(
        "##BlockProperties",
        ImVec2(0, -ImGui::GetFrameHeightWithSpacing()),
        true
    );

    ImGui::TextColored(
        ImVec4(1.0f, 1.0f, 0.0f, 1.0f),
        "Block: %s",
        m_selectedBlockName.c_str()
    );

    ImGui::Separator();

    auto &block = wld::BlockRegistry::get().getBlock(m_selectedBlockID);

    static std::string previousBlockName;
    static char blockNameBuffer[128] = "";

    if (previousBlockName != m_selectedBlockName) {
        strncpy(
            blockNameBuffer,
            m_selectedBlockName.c_str(),
            sizeof(blockNameBuffer) - 1
        );
        blockNameBuffer[sizeof(blockNameBuffer) - 1] = '\0';
        previousBlockName = m_selectedBlockName;
    }

    if (ImGui::InputText("Name", blockNameBuffer, sizeof(blockNameBuffer))) {
        std::string newName = blockNameBuffer;
        if (newName != m_selectedBlockName && !newName.empty()) {
            m_selectedBlock->name = newName;
            m_selectedBlockName = newName;
        }
    }

    int newID = m_selectedBlockID;
    if (ImGui::InputInt("ID", &newID)) {
        if (newID != m_selectedBlockID) {
            wld::BlockRegistry::get().changeBlockId(
                m_selectedBlockID,
                newID
            );
            m_selectedBlockID = newID;
        }
    }

    ImGui::Separator();

    ImGui::Checkbox("Transparency", &block.transparency);

    ImGui::SameLine();

    ImGui::Checkbox("Collision", &block.collision);

    ImGui::SameLine();

    ImGui::Checkbox("Breakable", &block.breakable);

    ImGui::Checkbox("Cross", &block.cross);

    ImGui::Separator();

    ImGui::Text("Light Emission:");
    
    ImGui::PushItemWidth(150.0f);
    ImGui::DragInt(
        "##Emission",
        reinterpret_cast<int*>(&block.emission),
        0.1f,
        0,
        15,
        "%d"
    );
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Determines light level (0-15)\nHigher values make block emit more light");
    ImGui::PopItemWidth();
    
    ImGui::SameLine();

    float emmVal = static_cast<float>(block.emission) / 15.0f;
    ImVec4 lightColor(emmVal, emmVal * 0.9f, emmVal * 0.7f, 1.0f);
    ImGui::ColorButton("##LightPreview", lightColor, 0, ImVec2(24, 24));
    
    ImGui::EndChild();
}

void BlockEditor::renderTextureSelector()
{
    if (m_selectedBlockName.empty()) {
        renderNoBlockSelected();
        return;
    }

    ImGui::BeginChild(
        "##TextureSelector",
        ImVec2(0, -1),
        true
    );

    renderTextureAtlas();

    ImGui::SameLine();

    renderCubeFaceSelector();

    ImGui::Text("Quick Texture Actions:");

    if (ImGui::Button("Set All")) {
        for (int i = 0; i < 6; i++) {
            m_selectedBlock->textures.fill(
                glm::uvec2(m_selectedTextureX, m_selectedTextureY)
            );
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Set Sides")) {
        m_selectedBlock->textures.sides(
            glm::uvec2(m_selectedTextureX, m_selectedTextureY)
        );
    }

    ImGui::EndChild();
}

void BlockEditor::renderTextureAtlas()
{
    ImGui::Text("Texture Atlas:");

    ImVec2 availSize = ImGui::GetContentRegionAvail();
    f32 displaySize = std::min(availSize.x * 0.4f, 300.0f);

    ImTextureID texture = reinterpret_cast<ImTextureID>(
        m_textureCache->getTexture("terrain")->descriptorSet
    );

    ImGui::BeginChild(
        "##AtlasView",
        ImVec2(displaySize, displaySize),
        true,
        ImGuiWindowFlags_HorizontalScrollbar
    );

    ImGui::Image(
        texture,
        ImVec2(displaySize, displaySize),
        ImVec2(0, 1),
        ImVec2(1, 0)
    );

    handleTextureAtlasInteraction(displaySize);

    ImGui::EndChild();
}

void BlockEditor::handleTextureAtlasInteraction(f32 displaySize)
{
    ImTextureID texture = reinterpret_cast<ImTextureID>(
        m_textureCache->getTexture("terrain")->descriptorSet
    );
    
    if (!texture || !ImGui::IsItemHovered())
        return;
        
    ImVec2 mousePos = ImGui::GetMousePos();
    ImVec2 imageMin = ImGui::GetItemRectMin();
    f32 normalizedX = (mousePos.x - imageMin.x) / displaySize;
    f32 normalizedY = (mousePos.y - imageMin.y) / displaySize;

    int tileX = static_cast<int>(normalizedX * 16);
    int tileY = static_cast<int>(normalizedY * 16);

    tileX = std::max(0, std::min(15, tileX));
    tileY = std::max(0, std::min(15, tileY));

    ImGui::BeginTooltip();
    ImGui::Text("Texture: x=%d, y=%d", tileX, tileY);
    ImGui::EndTooltip();

    if (ImGui::IsMouseClicked(0)) {
        m_selectedTextureX = tileX;
        m_selectedTextureY = tileY;
    }
}

void BlockEditor::renderCubeFaceSelector()
{
    ImTextureID texture = reinterpret_cast<ImTextureID>(
        m_textureCache->getTexture("terrain")->descriptorSet
    );
    
    ImVec2 availSize = ImGui::GetContentRegionAvail();
    f32 displaySize = std::min(availSize.x * 0.7f, 500.0f);

    if (m_selectedBlock->cross) {
        renderCrossBlockView(displaySize, texture);
    } else {
        renderStandardBlockView(displaySize, texture);
    }
}

void BlockEditor::renderCrossBlockView(f32 displaySize, ImTextureID texture)
{
    ImGui::BeginChild(
        "##UnfoldedView",
        ImVec2(0, displaySize),
        true,
        ImGuiWindowFlags_HorizontalScrollbar
    );

    const f32 faceSize = std::min(
        (ImGui::GetContentRegionAvail().x) / 4,
        displaySize / 4
    );

    ImGui::Text("Selected Texture: x=%d, y=%d", m_selectedTextureX, m_selectedTextureY);
    ImGui::Spacing();

    ImDrawList *drawList = ImGui::GetWindowDrawList();

    f32 startX = ImGui::GetCursorScreenPos().x + ImGui::GetContentRegionAvail().x * 0.25f;
    f32 startY = ImGui::GetCursorScreenPos().y + ImGui::GetStyle().ItemSpacing.y;

    ImU32 borderColor = IM_COL32(255, 255, 255, 255);
    ImU32 fillColor = IM_COL32(100, 100, 255, 200);

    auto getTexCoord = [&](wld::Face face) -> ImVec2 {
        glm::uvec2 texCoord = m_selectedBlock->textures.getUV(face);
        return ImVec2(texCoord.x / 16.0f, (15 - texCoord.y) / 16.0f);
    };

    ImVec2 crossUv = getTexCoord(wld::Face::NORTH);
    ImVec2 faceMin(startX, startY);
    ImVec2 faceMax(startX + faceSize * 2, startY + faceSize * 2);

    bool isHovered = ImGui::IsMouseHoveringRect(faceMin, faceMax);

    if (texture) {
        ImVec2 uvMin(crossUv.x, crossUv.y + 1.0f / 16.0f);
        ImVec2 uvMax(crossUv.x + 1.0f / 16.0f, crossUv.y);

        drawList->AddImage(
            texture,
            faceMin,
            faceMax,
            uvMin,
            uvMax,
            IM_COL32(255, 255, 255, 255)
        );
    }

    if (isHovered) {
        drawList->AddRectFilled(
            faceMin,
            faceMax,
            fillColor
        );
    }

    drawList->AddRect(
        faceMin,
        faceMax,
        borderColor,
        0.0f,
        ImDrawFlags_None,
        1.0f
    );

    const char* labelText = "CROSS TEXTURE";
    ImVec2 textSize = ImGui::CalcTextSize(labelText);
    ImVec2 textPos(
        faceMin.x + (faceSize * 2 - textSize.x) * 0.5f,
        faceMin.y + (faceSize * 2 - textSize.y) * 0.5f
    );

    drawList->AddText(
        textPos,
        IM_COL32(255, 255, 255, 255),
        labelText
    );

    if (
        isHovered &&
        ImGui::IsMouseClicked(0) &&
        m_selectedTextureX >= 0 &&
        m_selectedTextureY >= 0
    ) {
        m_selectedBlock->textures.fill(
            glm::uvec2(m_selectedTextureX, m_selectedTextureY)
        );
    }

    ImGui::Dummy(ImVec2(0, faceSize * 2.5f));

    ImGui::Separator();
    ImGui::TextWrapped(
        "Cross-type blocks use the same texture for all faces, "
        "displayed as two intersecting planes to simulate plants."
    );

    ImGui::EndChild();
}

void BlockEditor::renderStandardBlockView(f32 displaySize, ImTextureID texture)
{
    ImGui::BeginChild(
        "##UnfoldedView",
        ImVec2(0, displaySize),
        true,
        ImGuiWindowFlags_HorizontalScrollbar
    );

    const f32 faceSize = std::min(
        (ImGui::GetContentRegionAvail().x) / 4,
        displaySize / 4
    );

    ImGui::Text("Selected Texture: x=%d, y=%d", m_selectedTextureX, m_selectedTextureY);
    ImGui::Spacing();

    ImDrawList *drawList = ImGui::GetWindowDrawList();

    f32 startX = ImGui::GetCursorScreenPos().x + faceSize;
    f32 startY = ImGui::GetCursorScreenPos().y + ImGui::GetStyle().ItemSpacing.y;

    ImU32 borderColor = IM_COL32(255, 255, 255, 255);
    ImU32 fillColor = IM_COL32(100, 100, 255, 200);
    
    auto getTexCoord = [&](wld::Face face) -> ImVec2 {
        glm::uvec2 texCoord = m_selectedBlock->textures.getUV(face);
        return ImVec2(texCoord.x / 16.0f, (15 - texCoord.y) / 16.0f);
    };

    std::array<FaceInfo, 6> faces = {
        FaceInfo{wld::Face::TOP,    "TOP",    startX + faceSize,     startY,                getTexCoord(wld::Face::TOP)},
        FaceInfo{wld::Face::WEST,   "WEST",   startX,                startY + faceSize,     getTexCoord(wld::Face::WEST)},
        FaceInfo{wld::Face::NORTH,  "NORTH",  startX + faceSize,     startY + faceSize,     getTexCoord(wld::Face::NORTH)},
        FaceInfo{wld::Face::EAST,   "EAST",   startX + faceSize * 2, startY + faceSize,     getTexCoord(wld::Face::EAST)},
        FaceInfo{wld::Face::SOUTH,  "SOUTH",  startX + faceSize * 3, startY + faceSize,     getTexCoord(wld::Face::SOUTH)},
        FaceInfo{wld::Face::BOTTOM, "BOTTOM", startX + faceSize,     startY + faceSize * 2, getTexCoord(wld::Face::BOTTOM)}
    };

    renderFaces(faces, faceSize, drawList, texture, borderColor, fillColor);

    ImGui::Dummy(ImVec2(0, 3 * faceSize));
    ImGui::EndChild();
}

void BlockEditor::renderFaces(
    std::array<FaceInfo, 6> &faces, 
    f32 faceSize, 
    ImDrawList *drawList, 
    ImTextureID texture, 
    ImU32 borderColor, 
    ImU32 fillColor
)
{
    for (auto &face : faces) {
        ImVec2 faceMin(face.x, face.y);
        ImVec2 faceMax(face.x + faceSize, face.y + faceSize);

        bool isHovered = ImGui::IsMouseHoveringRect(faceMin, faceMax);

        if (texture) {
            ImVec2 uvMin(face.uv.x, face.uv.y + 1.0f / 16.0f);
            ImVec2 uvMax(face.uv.x + 1.0f / 16.0f, face.uv.y);

            drawList->AddImage(
                texture,
                faceMin,
                faceMax,
                uvMin,
                uvMax,
                fillColor
            );
        }

        if (isHovered) {
            drawList->AddRectFilled(
                faceMin,
                faceMax,
                fillColor
            );
        }

        drawList->AddRect(
            faceMin,
            faceMax,
            borderColor,
            0.0f,
            ImDrawFlags_None,
            1.0f
        );

        ImVec2 textSize = ImGui::CalcTextSize(face.name);
        ImVec2 textPos(
            faceMin.x + (faceSize - textSize.x) * 0.5f,
            faceMin.y + (faceSize - textSize.y) * 0.5f
        );

        drawList->AddText(
            textPos,
            IM_COL32(255, 255, 255, 255),
            face.name
        );

        if (
            isHovered &&
            ImGui::IsMouseClicked(0) &&
            m_selectedTextureX >= 0 &&
            m_selectedTextureY >= 0
        ) {
            m_selectedBlock->textures.set(
                face.face,
                glm::uvec2(m_selectedTextureX, m_selectedTextureY)
            );
        }
    }
}

void BlockEditor::renderSoundSelector()
{
    if (m_selectedBlockName.empty()) {
        renderNoBlockSelected();
        return;
    }

    ImGui::BeginChild(
        "##SoundSelector",
        ImVec2(0, -ImGui::GetFrameHeightWithSpacing()),
        true
    );

    ImGui::TextColored(
        ImVec4(1.0f, 1.0f, 0.0f, 1.0f),
        "Sound Settings for: %s",
        m_selectedBlockName.c_str()
    );

    ImGui::Separator();

    auto &allSounds = sfx::SoundManager::get().getSounds();

    std::map<std::string, sfx::Sound> breakSounds;
    std::map<std::string, sfx::Sound> stepSounds;
    std::map<std::string, sfx::Sound> placeSounds;

    for (const auto &[name, sound] : allSounds) {
        if (name.find("dig.") == 0) {
            breakSounds[name] = sound;
            placeSounds[name] = sound;
        } else if (name.find("step.") == 0) {
            stepSounds[name] = sound;
        }
    }

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 6));
    
    if (ImGui::CollapsingHeader("Break Sounds", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent(10.0f);
        
        if (breakSounds.empty()) {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No break sounds available");
        } else {
            const char* currentDigSound = m_selectedBlock->breakSound.sound.empty() ? 
                "No sound selected" : m_selectedBlock->breakSound.sound.c_str();
            
            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.7f);
            if (ImGui::BeginCombo("##BreakSound", currentDigSound)) {
                bool isNoneSelected = m_selectedBlock->breakSound.sound.empty();
                if (ImGui::Selectable("No sound selected", isNoneSelected)) {
                    m_selectedBlock->breakSound.sound = "";
                }
                
                if (isNoneSelected) {
                    ImGui::SetItemDefaultFocus();
                }
                
                for (const auto& [name, sound] : breakSounds) {
                    bool isSelected = (name == m_selectedBlock->breakSound.sound);
                    if (ImGui::Selectable(name.c_str(), isSelected)) {
                        m_selectedBlock->breakSound.sound = name;
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::PopItemWidth();
            
            ImGui::SameLine();
            ImGui::TextUnformatted("Sound:");
            
            ImGui::Spacing();
            float pitch = m_selectedBlock->breakSound.pitch;
            
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Pitch:");
            ImGui::SameLine();
            
            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.7f);
            if (ImGui::SliderFloat("##BreakPitch", &pitch, 0.5f, 2.0f, "%.2f")) {
                m_selectedBlock->breakSound.pitch = pitch;
            }
            ImGui::PopItemWidth();
            
            ImGui::SameLine();
            
            if (ImGui::Button("Play")) {
                if (!m_selectedBlock->breakSound.sound.empty()) {
                    sfx::SoundManager::get().play(
                        m_selectedBlock->breakSound.sound,
                        m_selectedBlock->breakSound.pitch,
                        false
                    );
                }
            }
        }
        ImGui::Unindent(10.0f);
    }
    
    if (ImGui::CollapsingHeader("Step Sounds")) {
        ImGui::Indent(10.0f);
        
        if (stepSounds.empty()) {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No step sounds available");
        } else {
            const char* currentStepSound = m_selectedBlock->stepSound.sound.empty() ? 
                "No sound selected" : m_selectedBlock->stepSound.sound.c_str();
            
            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.7f);
            if (ImGui::BeginCombo("##StepSound", currentStepSound)) {
                bool isNoneSelected = m_selectedBlock->stepSound.sound.empty();
                if (ImGui::Selectable("No sound selected", isNoneSelected)) {
                    m_selectedBlock->stepSound.sound = "";
                }
                
                if (isNoneSelected) {
                    ImGui::SetItemDefaultFocus();
                }
                
                for (const auto& [name, sound] : stepSounds) {
                    bool isSelected = (name == m_selectedBlock->stepSound.sound);
                    if (ImGui::Selectable(name.c_str(), isSelected)) {
                        m_selectedBlock->stepSound.sound = name;
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::PopItemWidth();
            
            ImGui::SameLine();
            ImGui::TextUnformatted("Sound:");
            
            ImGui::Spacing();
            float pitch = m_selectedBlock->stepSound.pitch;
            
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Pitch:");
            ImGui::SameLine();
            
            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.7f);
            if (ImGui::SliderFloat("##StepPitch", &pitch, 0.5f, 2.0f, "%.2f")) {
                m_selectedBlock->stepSound.pitch = pitch;
            }
            ImGui::PopItemWidth();
            
            ImGui::SameLine();
            
            if (ImGui::Button("Play##Step")) {
                if (!m_selectedBlock->stepSound.sound.empty()) {
                    sfx::SoundManager::get().play(
                        m_selectedBlock->stepSound.sound,
                        m_selectedBlock->stepSound.pitch,
                        false
                    );
                }
            }
        }
        ImGui::Unindent(10.0f);
    }
    
    if (ImGui::CollapsingHeader("Place Sounds")) {
        ImGui::Indent(10.0f);
        
        if (placeSounds.empty()) {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No place sounds available");
        } else {
            const char* currentPlaceSound = m_selectedBlock->placeSound.sound.empty() ? 
                "No sound selected" : m_selectedBlock->placeSound.sound.c_str();
            
            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.7f);
            if (ImGui::BeginCombo("##PlaceSound", currentPlaceSound)) {
                bool isNoneSelected = m_selectedBlock->placeSound.sound.empty();
                if (ImGui::Selectable("No sound selected", isNoneSelected)) {
                    m_selectedBlock->placeSound.sound = "";
                }
                
                if (isNoneSelected) {
                    ImGui::SetItemDefaultFocus();
                }
                
                for (const auto& [name, sound] : placeSounds) {
                    bool isSelected = (name == m_selectedBlock->placeSound.sound);
                    if (ImGui::Selectable(name.c_str(), isSelected)) {
                        m_selectedBlock->placeSound.sound = name;
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::PopItemWidth();
            
            ImGui::SameLine();
            ImGui::TextUnformatted("Sound:");
            
            ImGui::Spacing();
            float pitch = m_selectedBlock->placeSound.pitch;
            
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Pitch:");
            ImGui::SameLine();
            
            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.7f);
            if (ImGui::SliderFloat("##PlacePitch", &pitch, 0.5f, 2.0f, "%.2f")) {
                m_selectedBlock->placeSound.pitch = pitch;
            }
            ImGui::PopItemWidth();
            
            ImGui::SameLine();
            
            if (ImGui::Button("Play##Place")) {
                if (!m_selectedBlock->placeSound.sound.empty()) {
                    sfx::SoundManager::get().play(
                        m_selectedBlock->placeSound.sound,
                        m_selectedBlock->placeSound.pitch,
                        false
                    );
                }
            }
        }
        ImGui::Unindent(10.0f);
    }
    
    ImGui::PopStyleVar();

    ImGui::EndChild();
}

void BlockEditor::renderNoBlockSelected()
{
    ImGui::BeginChild("##NoElementSelected", ImVec2(0, -1), true);
        
    ImVec2 region = ImGui::GetContentRegionAvail();
    f32 centerPosY = region.y * 0.4f;
    
    ImGui::Dummy(ImVec2(0, centerPosY));

    f32 textWidth = ImGui::CalcTextSize("No Block Selected").x;
    f32 textPosX = (ImGui::GetWindowWidth() - textWidth) * 0.5f;
    
    ImGui::SetCursorPosX(textPosX);
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No Block Selected");
    
    ImGui::Spacing();
    ImGui::Spacing();

    const std::string text = "Select a block from the list or create a new one";
    
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

void BlockEditor::handleInput()
{
    bool ctrlPressed = ImGui::GetIO().KeyCtrl;

    if (ctrlPressed) {
        if (ImGui::IsKeyPressed(ImGuiKey_N)) {
            createNewBlock();
        } else if (ImGui::IsKeyPressed(ImGuiKey_S)) {
            wld::BlockRegistry::get().save();
        } else if (ImGui::IsKeyPressed(ImGuiKey_O)) {
            wld::BlockRegistry::get().load();
        } else if (ImGui::IsKeyPressed(ImGuiKey_D) && !m_selectedBlockName.empty()) {
            duplicateSelectedBlock();
        } else if (ImGui::IsKeyPressed(ImGuiKey_Delete) && !m_selectedBlockName.empty()) {
            deleteSelectedBlock();
        }
    }
}

void BlockEditor::createNewBlock()
{
    wld::Block newBlock;
    newBlock.name = "New Block";
    newBlock.textures.fill(glm::uvec2(0, 0));
    newBlock.breakSound.sound = "dig.stone";
    newBlock.breakSound.pitch = 1.0f;
    newBlock.stepSound.sound = "step.stone";
    newBlock.stepSound.pitch = 1.0f;
    newBlock.placeSound.sound = "dig.stone";
    newBlock.placeSound.pitch = 1.0f;

    m_selectedBlockID = wld::BlockRegistry::get().addBlock(newBlock);
    m_selectedBlockName = newBlock.name;
    m_selectedBlock = &wld::BlockRegistry::get().getBlock(m_selectedBlockID);
}

void BlockEditor::deleteSelectedBlock()
{
    if (m_selectedBlockName.empty()) {
        return;
    }

    wld::BlockRegistry::get().removeBlock(m_selectedBlockID);
    m_selectedBlockName.clear();
    m_selectedBlock = nullptr;
    m_selectedBlockID = 0;
}

void BlockEditor::duplicateSelectedBlock()
{
    if (m_selectedBlockName.empty()) {
        return;
    }

    wld::Block newBlock = *m_selectedBlock;
    newBlock.name = m_selectedBlockName + "_copy";

    m_selectedBlockID = wld::BlockRegistry::get().addBlock(newBlock);
    m_selectedBlockName = newBlock.name;
    m_selectedBlock = &wld::BlockRegistry::get().getBlock(m_selectedBlockID);
}

} // namespace adm