#pragma once

#include "core/camera/camera.hpp"
#include "graphics/device.hpp"
#include "graphics/pipeline.hpp"
#include "world/core/world.hpp"


namespace wld
{

class Outline
{

public:
    void init(gfx::Device &device, World &world);
    void destroy();

    void render(VkCommandBuffer cmd, const core::Camera &camera);

private:
    gfx::Pipeline m_pipeline;

    World *m_world;

    void drawOutline(
        VkCommandBuffer cmd,
        const glm::ivec3 &pos
    );
};

} // namespace wld