#pragma once

#include "vulkan_ctx.hpp"

namespace gfx
{

class UniformBuffer
{

public:
    void init(VulkanCtx &ctx, VkDeviceSize size);
    void destroy();

    void update(const void *data, VkDeviceSize size);

    VkBuffer getHandle() const { return m_buffer; }
    VkDeviceSize getSize() const { return m_size; }

private:
    VulkanCtx *m_ctx;
    
    VkBuffer m_buffer;
    VmaAllocation m_allocation;
    VkDeviceSize m_size;
};

} // namespace gfx