#include "uniform_buffer.hpp"

namespace gfx
{

void UniformBuffer::init(VulkanCtx &ctx, VkDeviceSize size)
{
    m_ctx = &ctx;
    m_size = size;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    VkResult res = vmaCreateBuffer(
        m_ctx->getAllocator(),
        &bufferInfo,
        &allocInfo,
        &m_buffer,
        &m_allocation,
        nullptr
    );

    VulkanCtx::check(res, "Failed to create uniform buffer");
}

void UniformBuffer::destroy()
{
    vmaDestroyBuffer(m_ctx->getAllocator(), m_buffer, m_allocation);
}

void UniformBuffer::update(const void *data, VkDeviceSize size)
{
    void *mapped;
    vmaMapMemory(m_ctx->getAllocator(), m_allocation, &mapped);
    memcpy(mapped, data, size);
    vmaUnmapMemory(m_ctx->getAllocator(), m_allocation);
}

} // namespace gfx