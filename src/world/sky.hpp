#pragma once

#include <glm/ext.hpp>

#include <array>

#include "graphics/vulkan_ctx.hpp"
#include "graphics/pipeline.hpp"
#include "core/camera/camera.hpp"

namespace wld
{

class Sky
{

public:
    void init(gfx::VulkanCtx &ctx);
    void destroy();

    void render(const core::Camera &camera);
private:
    gfx::VulkanCtx *m_ctx;
    gfx::Pipeline m_pipeline;

    struct Vertex
    {
        glm::vec3 pos;

        static VkVertexInputBindingDescription getBindingDescription()
        {
            VkVertexInputBindingDescription bindingDescription = {};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Vertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        }

        static std::array<VkVertexInputAttributeDescription, 1> getAttributeDescriptions()
        {
            std::array<VkVertexInputAttributeDescription, 1> attributeDescriptions = {};

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex, pos);

            return attributeDescriptions;
        }
    };

    VkBuffer m_vertexBuffer;
    VmaAllocation m_vertexAllocation;
    std::vector<Vertex> m_vertices;

    VkBuffer m_indexBuffer;
    VmaAllocation m_indexAllocation;
    std::vector<u32> m_indices;

    void createVertexBuffer();
    void createIndexBuffer();
};

} // namespace wld