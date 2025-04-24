#pragma once

#include <imgui.h>

#include "imgui_manager.hpp"

#include "graphics/texture_cache.hpp"
#include "world/blocks/block_registry.hpp"

namespace adm
{

struct FaceInfo {
    wld::Face face;
    const char *name;
    f32 x;
    f32 y;
    ImVec2 uv;
};

class BlockEditor
{

public:
    BlockEditor() = default;
    ~BlockEditor() = default;

    void init(gfx::TextureCache &textureCache);

    void render();

    bool isVisible() const { return m_visible; }
    void setVisible(bool visible) { m_visible = visible; }
    void toggleVisible() { m_visible = !m_visible; }

private:
    gfx::TextureCache *m_textureCache = nullptr;

    bool m_visible = false;

    std::string m_selectedBlockName = "";
    wld::Block *m_selectedBlock = nullptr;
    u32 m_selectedBlockID = 0;

    int m_selectedTextureX = 0;
    int m_selectedTextureY = 0;

    void renderBlockList();
    void renderBlockProperties();
    void renderTextureSelector();
    void renderTextureAtlas();
    void renderCubeFaceSelector();
    void renderCrossBlockView(f32 displaySize, ImTextureID texture);
    void renderStandardBlockView(f32 displaySize, ImTextureID texture);
    void renderFaces(
        std::array<FaceInfo, 6> &faces,
        f32 faceSize,
        ImDrawList *drawList,
        ImTextureID texture,
        ImU32 borderColor,
        ImU32 fillColor
    );
    void renderSoundSelector();
    void renderNoBlockSelected();

    void handleInput();
    void createNewBlock();
    void deleteSelectedBlock();
    void duplicateSelectedBlock();
    void handleTextureAtlasInteraction(f32 displaySize);

};

} // namespace adm