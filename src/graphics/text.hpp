#pragma once

#include "vulkan_ctx.hpp"
#include "pipeline.hpp"
#include "texture.hpp"

namespace gfx
{

class Text
{

public:
    void init(VulkanCtx &ctx);
    void destroy();

    void draw(
        const std::string &text,
        const glm::vec2 &pos,
        f32 size = 0.2
    );

private:
    VulkanCtx *m_ctx;

    struct PushConstant
    {
        glm::vec2 pos;
        glm::vec2 uv;
        f32 size;
    };

    Pipeline m_pipeline;
    Texture m_font;

    VkDescriptorSet m_descriptorSet;

    std::array<f32, 256> m_charWidths;
};

} // namespace gfx