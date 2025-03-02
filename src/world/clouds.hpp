#pragma once

#include <stb_image.h>

#include <string>
#include <vector>

#include "graphics/vulkan_ctx.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/uniform_buffer.hpp"
#include "core/camera/camera.hpp"
#include "core/frustum.hpp"

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

    struct CloudCell
    {
        int x, z;

        bool operator==(const CloudCell &other) const
        {
            return x == other.x && z == other.z;
        }
    };

    struct CloudCellHash
    {
        usize operator()(const CloudCell &cell) const
        {
            return std::hash<int>()(cell.x) ^ (std::hash<int>()(cell.z) << 1);
        }
    };

    std::unordered_map<
        CloudCell,
        std::vector<glm::vec2>,
        CloudCellHash
    > m_clouds;

    static constexpr int GRID_DIV = 4;

    void loadPattern(const std::string &path);
    void generateClouds();

    static constexpr f32 CLOUD_SIZE = 12.0f;

    static constexpr f32 WIND_SPEED = 1.5f;
    f32 m_windOffset = 0.0f;

    static constexpr f32 CLOUD_ALTITUDE = 112.0f;
};

} // namespace wld
