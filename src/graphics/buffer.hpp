#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <vector>
#include <stdexcept>

#include "core/types.hpp"

namespace gfx
{

class Device;

class Buffer
{

public:
    Buffer() = default;
    ~Buffer() = default;

    void destroy();

    bool isValid() const { return m_buffer != VK_NULL_HANDLE; }

    void *map();
    void unmap();

    template<typename T>
    void uploadData(const T *data, u32 count);

    template<typename T>
    void uploadData(const std::vector<T>& data);

    template<typename T, usize N>
    void uploadData(const std::array<T, N>& data);

public:
    VkBuffer getBuffer() const { return m_buffer; }
    VmaAllocation getAllocation() const { return m_allocation; }
    VkDeviceSize getSize() const { return m_size; }

private:
    friend class Device;

    Device *m_device = nullptr;

    VkBuffer m_buffer = VK_NULL_HANDLE;
    VmaAllocation m_allocation = VK_NULL_HANDLE;
    VkDeviceSize m_size = 0;

    void *m_data = nullptr;
    bool m_isMapped = false;

private:
    void init(
        Device &device,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VmaMemoryUsage memoryUsage
    );

};

template<typename T>
void Buffer::uploadData(const T *data, u32 count)
{
    if (m_buffer == VK_NULL_HANDLE) {
        return;
    }

    VkDeviceSize size = sizeof(T) * count;
    if (size > m_size) {
        throw std::runtime_error("Buffer size is too small for upload data.");
    }

    void *mappedData = map();
    memcpy(mappedData, data, size);
    unmap();
}

template<typename T>
void Buffer::uploadData(const std::vector<T> &data)
{
    uploadData(data.data(), static_cast<u32>(data.size()));
}

template<typename T, usize N>
void Buffer::uploadData(const std::array<T, N> &data)
{
    uploadData(data.data(), static_cast<u32>(data.size()));
}

} // namespace gfx