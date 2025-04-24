#pragma once

#include "core/types.hpp"
#include "graphics/device.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/texture_cache.hpp"
#include "world/blocks/block_registry.hpp"

namespace gfx
{

class ItemRenderer
{

public:
    void init(Device &device, TextureCache &textureCache);
    void destroy();

    void render(
        VkCommandBuffer cmd,
        wld::BlockType blockType,
        const glm::vec2 &position,
        f32 scale = 1.0f
    );

private:
    Device *m_device = nullptr;

    Pipeline m_pipeline;
    u32 m_textureID = ~0u;

    struct PushConstant
    {
        alignas(16) glm::mat4 model;
        alignas(8) glm::vec2 topUV;
        alignas(8) glm::vec2 bottomUV;
        alignas(8) glm::vec2 sideUV;
        alignas(4) u32 textureID;
    };
};

} // namespace gfx