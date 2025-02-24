#include "sky.hpp"

namespace wld
{

void Sky::init(gfx::VulkanCtx &ctx)
{
    m_ctx = &ctx;

    auto binding = Vertex::getBindingDescription();
    auto attributes = Vertex::getAttributeDescriptions();

    m_pipeline = gfx::Pipeline::Builder(*m_ctx)
        .setShader(gfx::ShaderType::VERTEX, "sky.vert.spv")
        .setShader(gfx::ShaderType::FRAGMENT, "sky.frag.spv")
        .setVertexInput(
            &binding,
            attributes.data(),
            attributes.size()
        )
        .addPushConstant(
            gfx::ShaderType::VERTEX,
            0,
            sizeof(glm::mat4)
        )
        .setDepthTest(false)
        .setDepthWrite(false)
        .setCullMode(VK_CULL_MODE_NONE)
        .build();

    createVertexBuffer();
    createIndexBuffer();
}

void Sky::destroy()
{
    m_pipeline.destroy();

    vmaDestroyBuffer(
        m_ctx->getAllocator(),
        m_vertexBuffer,
        m_vertexAllocation
    );

    vmaDestroyBuffer(
        m_ctx->getAllocator(),
        m_indexBuffer,
        m_indexAllocation
    );
}

void Sky::render(const core::Camera &camera)
{
    VkCommandBuffer commandBuffer = m_ctx->getCommandBuffer();

    m_pipeline.bind();

    glm::mat4 view = glm::mat4(glm::mat3(camera.getView()));
    glm::mat4 viewProj = camera.getProj() * view;

    m_pipeline.push(
        gfx::ShaderType::VERTEX,
        0,
        sizeof(glm::mat4),
        &viewProj
    );

    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(
        commandBuffer,
        0,
        1,
        &m_vertexBuffer,
        offsets
    );

    vkCmdBindIndexBuffer(
        commandBuffer,
        m_indexBuffer,
        0,
        VK_INDEX_TYPE_UINT32
    );

    vkCmdDrawIndexed(
        commandBuffer,
        static_cast<u32>(m_indices.size()),
        1,
        0,
        0,
        0
    );
}

void Sky::createVertexBuffer()
{
    m_vertices = {
        {{-1.0f, -1.0f, -1.0f}},
        {{1.0f, -1.0f, -1.0f}},
        {{1.0f, 1.0f, -1.0f}},
        {{-1.0f, 1.0f, -1.0f}},
        {{-1.0f, -1.0f, 1.0f}},
        {{1.0f, -1.0f, 1.0f}},
        {{1.0f, 1.0f, 1.0f}},
        {{-1.0f, 1.0f, 1.0f}}
    };

    VkDeviceSize size = sizeof(m_vertices[0]) * m_vertices.size();

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    vmaCreateBuffer(
        m_ctx->getAllocator(),
        &bufferInfo,
        &allocInfo,
        &m_vertexBuffer,
        &m_vertexAllocation,
        nullptr
    );

    void *data;
    vmaMapMemory(m_ctx->getAllocator(), m_vertexAllocation, &data);
    memcpy(data, m_vertices.data(), size);
    vmaUnmapMemory(m_ctx->getAllocator(), m_vertexAllocation);
}

void Sky::createIndexBuffer()
{
    m_indices = {
        0, 1, 2,
        2, 3, 0,
        4, 5, 6,
        6, 7, 4,
        1, 5, 6,
        6, 2, 1,
        4, 0, 3,
        3, 7, 4,
        3, 2, 6,
        6, 7, 3,
        4, 5, 1,
        1, 0, 4
    };

    VkDeviceSize size = sizeof(m_indices[0]) * m_indices.size();

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    vmaCreateBuffer(
        m_ctx->getAllocator(),
        &bufferInfo,
        &allocInfo,
        &m_indexBuffer,
        &m_indexAllocation,
        nullptr
    );

    void *data;
    vmaMapMemory(m_ctx->getAllocator(), m_indexAllocation, &data);
    memcpy(data, m_indices.data(), size);
    vmaUnmapMemory(m_ctx->getAllocator(), m_indexAllocation);
}

} // namespace wld