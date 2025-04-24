#pragma once

#include <glm/ext.hpp>

#include <array>

#include "graphics/device.hpp"
#include "graphics/pipeline.hpp"
#include "core/camera/camera.hpp"

namespace wld
{

class Sky
{

public:
    void init(gfx::Device &device);
    void destroy();

    void render(VkCommandBuffer cmd);
private:
    gfx::Pipeline m_pipeline;

    VkDescriptorSet m_descriptorSet;
};

} // namespace wld