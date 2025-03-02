#pragma once

#include <stb_image.h>

#include "graphics/vulkan_ctx.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/texture.hpp"
#include "graphics/uniform_buffer.hpp"

namespace gui
{

enum class TextAlign
{
    LEFT,
    CENTER,
    RIGHT
};

class TextRenderer
{

public:
    void init(gfx::VulkanCtx &ctx);
    void destroy();

    void draw(
        const std::string &text,
        const glm::vec2 &pos,
        u32 size = 16,
        TextAlign align = TextAlign::LEFT
    );

private:
    gfx::VulkanCtx *m_ctx;

    struct UniformBufferObject
    {
        glm::mat4 ortho;
    };

    struct PushConstant
    {
        alignas(16) glm::mat4 model;
        alignas(16) glm::vec2 uv;
        alignas(16) glm::vec4 color;
    };

    gfx::Pipeline m_pipeline;
    gfx::Texture m_font;
    gfx::UniformBuffer m_ubo;

    VkDescriptorSet m_descriptorSet;

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