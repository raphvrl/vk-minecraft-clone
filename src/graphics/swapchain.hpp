#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "core/types.hpp"

#include "utils/init.hpp"
#include "global.hpp"

namespace gfx
{

class Swapchain
{

public:
    Swapchain() = default;
    ~Swapchain() = default;

    void init(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        VkSurfaceKHR surface,
        u32 width,
        u32 height
    );

    void destroy();

    void recreate(u32 width, u32 height);

    void beginFrame(u32 &currentFrame);
    std::pair<u32, VkImage> acquireNextImage(u32 currentFrame);
    void submit(u32 currentFrame,
        VkCommandBuffer commandBuffer,
        VkQueue graphicsQueue
    );
    void present(u32 currentFrame, VkQueue presentQueue);

public:
    VkSwapchainKHR getSwapchain() const { return m_swapchain; }
    VkImage getImage(u32 index) const { return m_images[index]; }
    VkImageView getImageView(u32 index) const { return m_imageViews[index]; }
    VkFormat getFormat() const { return m_imageFormat; }
    VkExtent2D getExtent() const { return m_extent; }
    bool isOutOfDate() const { return m_outOfDate; }

private:
    struct FrameData
    {
        VkSemaphore imageSemaphore = VK_NULL_HANDLE;
        VkSemaphore renderSemaphore = VK_NULL_HANDLE;
        VkFence renderFence = VK_NULL_HANDLE;
    };

private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;

    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_imageViews;

    VkFormat m_imageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D m_extent = {0, 0};

    std::vector<FrameData> m_frames;

    bool m_outOfDate = false;
    u32 m_imageIndex = 0;

    void cleanupSwapchain();
};

} // namespace gfx