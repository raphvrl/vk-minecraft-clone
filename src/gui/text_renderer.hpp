#pragma once

#include <stb_image.h>

#include "graphics/device.hpp"
#include "graphics/pipeline.hpp"

namespace gui
{

enum class TextAlign
{
    LEFT,
    RIGHT,
    CENTER,
};

class TextRenderer
{

public:
    void init(gfx::Device &device);
    void destroy();

    void draw(
        VkCommandBuffer cmd,
        const std::string &text,
        const glm::vec2 &pos,
        u32 size = 16,
        TextAlign align = TextAlign::LEFT
    );

private:
    gfx::Device *m_device = nullptr;

    struct PushConstant
    {
        alignas(16) glm::mat4 model;
        alignas(16) glm::vec2 uv;
        alignas(16) glm::vec4 color;
        alignas(4) u32 textureID;
    };

    gfx::Pipeline m_pipeline;

    gfx::Image m_texture;
    u32 m_textureID = 0;

    std::array<u32, 256> m_charWidths;

    void calculateCharWidths();
    int getCharWidth(
        stbi_uc *pixels,
        int imgWidth,
        int imgHeight,
        int channels,
        int startX,
        int startY,
        int charWidth,
        int charHeight
    );
};

} // namespace gfx