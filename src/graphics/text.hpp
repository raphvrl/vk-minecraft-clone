#pragma once

#include "vulkan_ctx.hpp"
#include "pipeline.hpp"
#include "texture.hpp"
#include "uniform_buffer.hpp"

namespace gfx
{

enum class TextAlign
{
    LEFT,
    CENTER,
    RIGHT
};

class Text
{

public:
    void init(VulkanCtx &ctx);
    void destroy();

    void draw(
        const std::string &text,
        const glm::vec2 &pos,
        u32 size = 16,
        TextAlign align = TextAlign::LEFT
    );

private:
    VulkanCtx *m_ctx;

    struct UniformBufferObject
    {
        glm::mat4 proj;
    };

    struct PushConstant
    {
        alignas(16) glm::mat4 model;
        alignas(16) glm::vec2 uv;
        alignas(16) glm::vec4 color;
    };

    Pipeline m_pipeline;
    Texture m_font;
    UniformBuffer m_ubo;

    VkDescriptorSet m_descriptorSet;

    std::array<u32, 256> m_charWidths;
};

} // namespace gfx