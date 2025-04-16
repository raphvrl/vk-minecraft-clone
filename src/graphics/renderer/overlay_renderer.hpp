#pragma once

#include "graphics/pipeline.hpp"
#include "graphics/device.hpp"
#include "graphics/texture_cache.hpp"

namespace gfx
{

class OverlayRenderer
{

public:
    void init(Device &device, TextureCache &textureCache);
    void destroy();

    void render(VkCommandBuffer cmd);

    void setWater(bool water) { m_water = water; }

private:
    Device *m_device = nullptr;

    Pipeline m_pipeline;

    u32 m_textureID = 0;

    bool m_water = false;

};

} // namespace gfx