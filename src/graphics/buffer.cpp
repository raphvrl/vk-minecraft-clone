#include "buffer.hpp"
#include "device.hpp"

namespace gfx
{

void Buffer::init(
    Device &device,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VmaMemoryUsage memoryUsage
)
{
    m_device = &device;
    m_size = size;

    if (m_size == 0) {
        m_size = 0;
        m_buffer = VK_NULL_HANDLE;
        m_allocation = VK_NULL_HANDLE;
        return;
    }

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = memoryUsage;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationInfo allocationInfo{};
    
    VkResult res = vmaCreateBuffer(
        m_device->getAllocator(),
        &bufferInfo,
        &allocInfo,
        &m_buffer,
        &m_allocation,
        &allocationInfo
    );

    vk::check(res, "Failed to create buffer!");
}

void Buffer::destroy()
{
    if (m_buffer == VK_NULL_HANDLE) {
        return;
    }

    if (m_isMapped) {
        unmap();
    }

    vmaDestroyBuffer(
        m_device->getAllocator(),
        m_buffer,
        m_allocation
    );
}

void *Buffer::map()
{
    if (m_buffer == VK_NULL_HANDLE) {
        return nullptr;
    }

    if (m_isMapped) {
        return m_data;
    }

    VkResult res = vmaMapMemory(
        m_device->getAllocator(),
        m_allocation,
        &m_data
    );

    vk::check(res, "Failed to map buffer!");

    m_isMapped = true;
    return m_data;
}

void Buffer::unmap()
{
    if (!m_isMapped) {
        return;
    }

    vmaUnmapMemory(m_device->getAllocator(), m_allocation);
    m_isMapped = false;
}

} // namespace gfx