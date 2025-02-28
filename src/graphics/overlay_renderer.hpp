#pragma once

#include "pipeline.hpp"
#include "texture.hpp"
#include "vulkan_ctx.hpp"

namespace gfx
{

class OverlayRenderer
{

public:
    void init(VulkanCtx &ctx);
    void destroy();

    void render();

    void setWater(bool water) { m_water = water; }

private:
    VulkanCtx *m_ctx = nullptr;

    Pipeline m_pipeline;
    Texture m_waterTexture;

    VkDescriptorSet m_waterSet;

    bool m_water = false;

    void createWater();

};

} // namespace gfx