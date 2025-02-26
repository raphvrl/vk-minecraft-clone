#pragma once

#include <stb_image.h>

#include <string>
#include <vector>

#include "graphics/vulkan_ctx.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/uniform_buffer.hpp"
#include "core/camera/camera.hpp"

namespace wld
{

class Clouds
{

public:
    void init(gfx::VulkanCtx &ctx);
    void destroy();

    void update(f32 dt);
    void render(const core::Camera &camera);

private:
    gfx::VulkanCtx *m_ctx;
    gfx::Pipeline m_pipeline;

    struct UniformBufferObject
    {
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
        alignas(16) glm::vec3 camPos;
    };

    gfx::UniformBuffer m_ubo;
    VkDescriptorSet m_descriptorSet;

    i32 m_width;
    i32 m_height;
    std::vector<bool> m_pattern;

    void loadPattern(const std::string &path);

    std::vector<glm::vec2> m_clouds;

    void generateClouds();

    static constexpr f32 CLOUD_SIZE = 12.0f;

    static constexpr f32 WIND_SPEED = 1.5f;
    f32 m_windOffset = 0.0f;

    static constexpr f32 CLOUD_ALTITUDE = 112.0f;
};

} // namespace wld
