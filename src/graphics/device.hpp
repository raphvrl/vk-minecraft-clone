#pragma once

#include "core/window/window.hpp"
#include "utils/init.hpp"
#include "global.hpp"
#include "swapchain.hpp"
#include "buffer.hpp"
#include "depth_buffer.hpp"
#include "bindless_manager.hpp"

namespace gfx
{

class Device
{

public:
    Device() = default;
    ~Device() = default;

    void init(
        core::Window &window,
        const std::string &appName,
        const vk::Version &version
    );

    void destroy();

    VkCommandBuffer beginFrame();
    void endFrame();

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    VkSampler createSampler(
        VkFilter magFilter,
        VkFilter minFilter,
        VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        f32 maxAnisotropy = 1.0f
    );

    Buffer createBuffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
    );

    Image createImage(
        u32 width,
        u32 height,
        VkFormat format,
        VkImageUsageFlags usage,
        VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT,
        u32 mipLevels = 1,
        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT,
        VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO,
        bool createView = true
    );

    Image loadImage(
        const std::string& filepath,
        VkFormat format = VK_FORMAT_R8G8B8A8_SRGB,
        VkImageUsageFlags additionalUsage = 0,
        bool mipmaps = true,
        VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT
    );

    Image createImageFromData(
        const void* data,
        u32 width,
        u32 height,
        VkFormat format = VK_FORMAT_R8G8B8A8_SRGB,
        VkImageUsageFlags additionalUsage = 0,
        bool mipmaps = true,
        VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT
    );

    u32 addUBO(
        const Buffer &buffer,
        VkDeviceSize offset = 0,
        VkDeviceSize range = 0
    ) {
        return m_bindlessManager.addUBO(buffer, offset, range);
    }

    u32 addSSBO(
        const Buffer &buffer,
        VkDeviceSize offset = 0,
        VkDeviceSize range = 0
    ) {
        return m_bindlessManager.addSSBO(buffer, offset, range);
    }

    u32 addTexture(
        const Image &image,
        VkSampler sampler = VK_NULL_HANDLE
    ) {
        return m_bindlessManager.addTexture(image, sampler);
    }

    void update()
    {
        m_bindlessManager.update();
    }

    void waitIdle();

public:
    VkInstance getInstance() const { return m_instance; }
    VkPhysicalDevice getPhysicalDevice() const { return m_physicalDevice; }
    VkDevice getDevice() const { return m_device; }

    VkSurfaceKHR getSurface() const { return m_surface; }

    Swapchain &getSwapchain() { return m_swapchain; }
    VkExtent2D getExtent() const { return m_swapchain.getExtent(); }

    VmaAllocator getAllocator() const { return m_allocator; }

    DepthBuffer &getDepthBuffer() { return m_depthBuffer; }
    VkFormat getDepthFormat() const { return m_depthBuffer.getFormat(); }

    VkSampler getDefaultSampler() const { return m_defaultSampler; }

    BindlessManager &getBindlessManager() { return m_bindlessManager; }

    VkQueue getGraphicsQueue() const { return m_graphicsQueue; }
    VkQueue getPresentQueue() const { return m_presentQueue; }

private:
    struct FrameData
    {
        VkCommandPool commandPool = VK_NULL_HANDLE;
        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    };

private:
    core::Window *m_window = nullptr;

    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;

    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    Swapchain m_swapchain;
    VmaAllocator m_allocator = VK_NULL_HANDLE;
    DepthBuffer m_depthBuffer;

    VkSampler m_defaultSampler = VK_NULL_HANDLE;

    BindlessManager m_bindlessManager;

    vk::QueueFamilyIndices m_queueFamilyIndices;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;

    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;

    std::array<FrameData, MAX_FRAMES_IN_FLIGHT> m_frames;
    u32 m_currentFrame = 0;
    u32 m_imageIndex = 0;

private:
    void recreateSwapchain();

};


} // namespace gfx