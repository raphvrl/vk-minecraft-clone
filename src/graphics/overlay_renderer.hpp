#pragma once

#include "pipeline.hpp"
#include "device.hpp"

namespace gfx
{

class OverlayRenderer
{

public:
    void init(Device &device);
    void destroy();

    void render(VkCommandBuffer cmd);

    void setWater(bool water) { m_water = water; }

private:
    Device *m_device = nullptr;

    Pipeline m_pipeline;

    gfx::Image m_texture;
    u32 m_textureID = 0;

    bool m_water = false;

};

} // namespace gfx