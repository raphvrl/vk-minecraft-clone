#include "gui_editor.hpp"

namespace adm
{

void GUIEditor::init(
    ImGuiManager &imguiManager,
    gui::GUIManager &guiManager,
    gfx::TextureCache &textureCache
)
{
    m_imguiManager = &imguiManager;
    m_guiManager = &guiManager;
    m_textureCache = &textureCache;
}

void GUIEditor::render()
{
    if (!m_visible) return;

    ImGui::Begin("GUI", &m_visible);

    if (ImGui::Button("Save")) {
        m_guiManager->saveConfig();
    }

    ImGui::SameLine();

    if (ImGui::Button("Load")) {
        m_guiManager->loadConfig();
    }

    ImGui::SameLine();

    if (ImGui::Button("Delete")) {
        deleteSelectedElement();
    }

    ImGui::SameLine();

    if (ImGui::Button("Duplicate")) {
        duplicateSelectedElement();
    }

    ImGui::Separator();

    const f32 windowWidth = ImGui::GetWindowWidth();
    const f32 listWidth = windowWidth * 0.3f;
    const f32 propertiesWidth = windowWidth * 0.6f;

    ImGui::Columns(2, "##Columns", true);

    ImGui::SetColumnWidth(0, listWidth);
    renderElementList();

    ImGui::NextColumn();

    if (ImGui::BeginTabBar("PropertiesTabs")) {
        if (ImGui::BeginTabItem("Properties")) {
            renderElementProperties();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Textures")) {
            renderTextureSelector();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::Columns(1);

    ImGui::End();
}

void GUIEditor::renderElementList()
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

    if (m_guiManager->getElements().empty()) {
        ImGui::TextColored(
            ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
            "No elements found."
        );
        return;
    }

    ImGui::BeginChild(
        "##ElementsList",
        ImVec2(0, -ImGui::GetFrameHeightWithSpacing()),
        true
    );

    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.4f, 0.8f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.5f, 0.9f, 0.7f));

    ImVec4 selectedTextColor(1.0f, 1.0f, 0.0f, 1.0f);
    
    bool elementFound = false;
    for (const auto &[id, element] : m_guiManager->getElements()) {
        std::string lowerName = id;
        std::transform(
            lowerName.begin(),
            lowerName.end(),
            lowerName.begin(),
            [](unsigned char c) { return std::tolower(c); }
        );

        if (!searchStr.empty() && lowerName.find(searchStr) == std::string::npos) {
            continue;
        }

        elementFound = true;

        ImGui::PushID(id.c_str());

        bool isVisible = element->visible;
        ImGui::Checkbox("##visible", &isVisible);
        if (isVisible != element->visible) {
            element->visible = isVisible;
        }

        ImGui::SameLine();

        bool isSelected = (m_selectedElementID == id);
        if (isSelected) {
            ImGui::PushStyleColor(ImGuiCol_Text, selectedTextColor);
        }

        if (ImGui::Selectable(id.c_str(), isSelected)) {
            m_selectedElementID = id;
            m_selectedElement = element;
        }

        if (isSelected) {
            ImGui::PopStyleColor(1);
        }

        ImGui::PopID();
    }
    
    ImGui::PopStyleColor(2);

    ImGui::EndChild();

    if (ImGui::Button("+ Add Element", ImVec2(-1, 0))) {
        createNewElement();
    }
}

void GUIEditor::renderElementProperties()
{
    if (!m_selectedElement) {
        renderNoElementSelected();
        return;
    }

    ImGui::BeginChild(
        "##ElementProperties",
        ImVec2(0, -1),
        true
    );

    ImGui::TextColored(
        ImVec4(1.0f, 1.0f, 0.0f, 1.0f),
        "Element: %s",
        m_selectedElementID.c_str()
    );

    static std::string previousElementID;
    static char idBuffer[128] = "";
    
    if (previousElementID != m_selectedElementID) {
        strncpy(idBuffer, m_selectedElementID.c_str(), sizeof(idBuffer) - 1);
        idBuffer[sizeof(idBuffer) - 1] = '\0';
        previousElementID = m_selectedElementID;
    }
    
    if (ImGui::InputText("ID", idBuffer, sizeof(idBuffer))) {
        std::string newID = idBuffer;
        if (newID != m_selectedElementID && !newID.empty()) {
            m_guiManager->renameElement(m_selectedElementID, newID);
            m_selectedElementID = newID;
            m_selectedElement = m_guiManager->getElement(newID);
        }
    }

    const char *types[] = {"Panel", "Button", "Label", "Image"};
    static int selectedType = 0;
    ImGui::Combo("Type", &selectedType, types, IM_ARRAYSIZE(types));

    renderAnchorSelector();

    ImGui::Separator();

    ImGui::DragFloat2("Position", &m_selectedElement->position[0], 0.1f);
    ImGui::DragFloat2("Size", &m_selectedElement->size[0], 0.1f);

    ImGui::Separator();

    ImGui::Checkbox("Visible", &m_selectedElement->visible);

    ImGui::EndChild();
}

void GUIEditor::renderAnchorSelector()
{
    static const char *anchors[] = {
        "TL", "TC", "TR",
        "CL", "C", "CR",
        "BL", "BC", "BR"
    };

    ImGui::Text("Anchor:");

    const f32 buttonSize = 30.0f;
    const f32 spacing = 1.0f;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

    ImVec2 startPos = ImGui::GetCursorPos();

    ImGui::BeginGroup();

    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            f32 posX = x * (buttonSize + spacing);
            f32 posY = y * (buttonSize + spacing);

            ImGui::SetCursorPos(ImVec2(startPos.x + posX, startPos.y + posY));
            
            int index = y * 3 + x;
            ImGui::PushID(index);

            bool isActive = (static_cast<int>(m_selectedElement->anchor) == index);

            if (isActive) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.8f, 0.0f, 1.0f));
            }
            
            ImGui::Button(anchors[index], ImVec2(buttonSize, buttonSize));

            if (ImGui::IsItemClicked()) {
                m_selectedElement->anchor = static_cast<gui::ElementAnchor>(index);
            }
            
            if (isActive) {
                ImGui::PopStyleColor();
            }
            
            ImGui::PopID();
        }
    }

    f32 totalGridHeight = 3 * buttonSize + 2 * spacing;
    f32 totalGridWidth = 3 * buttonSize + 2 * spacing;
    ImGui::Dummy(ImVec2(totalGridWidth, 0));

    ImGui::PopStyleVar(2);
    
    ImGui::EndGroup();
}

void GUIEditor::renderTextureSelector()
{
    if (!m_selectedElement) {
        renderNoElementSelected();
        return;
    }

    ImGui::BeginChild(
        "##TextureSelector",
        ImVec2(0, -1),
        true
    );

    std::vector<std::string> textureNames = m_textureCache->getTextureNames();

    static std::string lastElementID;
    static char textureBuffer[256] = "";

    if (lastElementID != m_selectedElementID) {
        strncpy(
            textureBuffer, 
            m_selectedElement->texture.c_str(),
            sizeof(textureBuffer) - 1
        );
        textureBuffer[sizeof(textureBuffer) - 1] = '\0';
        lastElementID = m_selectedElementID;
    }

    int currentItemIndex = 0;
    for (size_t i = 0; i < textureNames.size(); i++) {
        if (textureNames[i] == textureBuffer) {
            currentItemIndex = static_cast<int>(i);
            break;
        }
    }

    std::vector<const char*> textureNamesCStr;
    for (const auto& name : textureNames) {
        textureNamesCStr.push_back(name.c_str());
    }

    if (ImGui::Combo(
        "Texture",
        &currentItemIndex,
        textureNamesCStr.data(), 
        static_cast<int>(textureNamesCStr.size()))
    ) {
        strncpy(
            textureBuffer,
            textureNames[currentItemIndex].c_str(),
            sizeof(textureBuffer) - 1
        );
        textureBuffer[sizeof(textureBuffer) - 1] = '\0';
        m_selectedElement->texture = textureBuffer;
    }
    
    ImGui::Separator();

    f32 uvPixels[4] = {
        m_selectedElement->uv.x, 
        m_selectedElement->uv.y,
        m_selectedElement->uv.z,
        m_selectedElement->uv.w
    };
    
    ImGui::Text("UV Coordinates (in pixels):");
    bool uvChanged = false;
    
    uvChanged |= ImGui::DragFloat2("UV0 (x, y)", &uvPixels[0], 1.0f, 0.0f, 256.0f);
    uvChanged |= ImGui::DragFloat2("UV1 (x, y)", &uvPixels[2], 1.0f, 0.0f, 256.0f);

    if (uvChanged) {
        m_selectedElement->uv = {
            uvPixels[0],
            uvPixels[1],
            uvPixels[2],
            uvPixels[3]
        };
    }
    
    ImGui::Separator();

    const std::string &textureName = textureBuffer;
    if (!textureName.empty()) {
        gfx::Texture *texture = m_textureCache->getTexture(textureName);
        
        if (texture) {
            ImGui::Text("Dimensions: %dx%d", texture->width, texture->height);

            const f32 previewWidth = ImGui::GetContentRegionAvail().x * 0.9f;
            const f32 previewHeight = 200.0f;
            
            ImGui::Text("Preview:");

            f32 texRatio = static_cast<f32>(texture->width) / texture->height;
            f32 previewRatio = previewWidth / previewHeight;
            
            f32 displayWidth, displayHeight;
            if (texRatio > previewRatio) {
                displayWidth = previewWidth;
                displayHeight = previewWidth / texRatio;
            } else {
                displayHeight = previewHeight;
                displayWidth = previewHeight * texRatio;
            }
            
            ImVec2 displayPos = ImGui::GetCursorPos();
            ImVec2 screenPos = ImGui::GetCursorScreenPos();

            ImGui::Image(
                reinterpret_cast<ImTextureID>(texture->descriptorSet),
                ImVec2(displayWidth, displayHeight),
                ImVec2(0, 1),
                ImVec2(1, 0)
            );

            ImDrawList* drawList = ImGui::GetWindowDrawList();

            f32 uvX0 = uvPixels[0] / texture->width;
            f32 uvY0 = uvPixels[1] / texture->height;
            f32 uvX1 = uvPixels[2] / texture->width;
            f32 uvY1 = uvPixels[3] / texture->height;
            
            ImVec2 uvRectMin(
                screenPos.x + uvX0 * displayWidth,
                screenPos.y + uvY0 * displayHeight
            );
            ImVec2 uvRectMax(
                screenPos.x + uvX1 * displayWidth,
                screenPos.y + uvY1 * displayHeight
            );

            ImVec2 finalMin = ImVec2(
                std::min(uvRectMin.x, uvRectMax.x),
                std::min(uvRectMin.y, uvRectMax.y)
            );
            ImVec2 finalMax = ImVec2(
                std::max(uvRectMin.x, uvRectMax.x),
                std::max(uvRectMin.y, uvRectMax.y)
            );

            drawList->AddRect(
                finalMin,
                finalMax,
                IM_COL32(255, 255, 0, 255),
                0.0f,
                0,
                2.0f
            );
        } else {
            ImGui::TextColored(
                ImVec4(1.0f, 0.0f, 0.0f, 1.0f),
                "Texture not found: %s",
                textureName.c_str()
            );
        }
    } else {
        ImGui::TextColored(
            ImVec4(1.0f, 0.5f, 0.0f, 1.0f),
            "No texture selected"
        );
    }

    ImGui::EndChild();
}

void GUIEditor::renderNoElementSelected()
{
    ImGui::BeginChild("##NoElementSelected", ImVec2(0, -1), true);
        
    ImVec2 region = ImGui::GetContentRegionAvail();
    float centerPosY = region.y * 0.4f;
    
    ImGui::Dummy(ImVec2(0, centerPosY));

    float textWidth = ImGui::CalcTextSize("No Element Selected").x;
    float textPosX = (ImGui::GetWindowWidth() - textWidth) * 0.5f;
    
    ImGui::SetCursorPosX(textPosX);
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No Element Selected");
    
    ImGui::Spacing();
    ImGui::Spacing();
    
    textWidth = ImGui::CalcTextSize(
        "Select an element from the list or create a new one"
    ).x;
    textPosX = (ImGui::GetWindowWidth() - textWidth) * 0.5f;
    
    ImGui::SetCursorPosX(textPosX);
    ImGui::TextColored(
        ImVec4(0.5f, 0.5f, 0.5f, 1.0f), 
        "Select an element from the list or create a new one"
    );
    
    ImGui::EndChild();
    return;
}

void GUIEditor::createNewElement()
{
    std::string baseId = "NewElement";
    std::string newId = baseId;
    int counter = 1;

    const auto &elements = m_guiManager->getElements();
    while (elements.find(newId) != elements.end()) {
        newId = baseId + std::to_string(counter);
        counter++;
    }

    auto newElement = std::make_shared<gui::GUIElement>();
    newElement->id = newId;
    newElement->type = gui::ElementType::PANEL;
    newElement->anchor = gui::ElementAnchor::TOP_LEFT;
    newElement->position = {0.0f, 0.0f};
    newElement->size = {100.0f, 100.0f};
    newElement->visible = true;
    newElement->uv = {0.0f, 0.0f, 255.0f, 255.0f};
    newElement->texture = "terrain.png";
    newElement->text = "";


    m_guiManager->addElement(newElement);

    m_selectedElementID = newId;
    m_selectedElement = newElement;
}

void GUIEditor::deleteSelectedElement()
{
    if (m_selectedElement) {
        m_guiManager->removeElement(m_selectedElementID);
        m_selectedElement = nullptr;
        m_selectedElementID = "";
    }
}

void GUIEditor::duplicateSelectedElement()
{
    if (m_selectedElement) {
        auto newElement = std::make_shared<gui::GUIElement>(*m_selectedElement);
        newElement->id = m_selectedElementID + "_copy";
        m_guiManager->addElement(newElement);
    }
}

} // namespace adm