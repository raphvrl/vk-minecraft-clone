#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <string>
#include <vector>
#include <optional>
#include <map>
#include <set>

#include "core/window/window.hpp"
#include "core/logger/logger.hpp"

namespace gfx
{
    
class VulkanCtx
{

public:
    VulkanCtx() = default;
    virtual ~VulkanCtx() = default;

    VulkanCtx(const VulkanCtx &) = delete;
    VulkanCtx &operator=(const VulkanCtx &) = delete;

    void init(core::Window &window);
    void destroy();

    // rendering
    bool beginFrame();
    void endFrame();

    // allocator
    void createBuffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VmaMemoryUsage memoryUsage,
        VkBuffer &buffer,
        VmaAllocation &allocation
    );

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    void createImage(
        u32 width,
        u32 height,
        VkFormat format,
        VkImageTiling tiling,
        VkImageUsageFlags usage,
        VmaMemoryUsage memoryUsage,
        VkImage &image,
        VmaAllocation &allocation
    );

    void transitionImageLayout(
        VkImage image,
        VkFormat format,
        VkImageLayout oldLayout,
        VkImageLayout newLayout
    );

    void copyBufferToImage(
        VkBuffer buffer,
        VkImage image,
        u32 width,
        u32 height
    );

    // getters
    VkInstance getInstance() { return m_instance; }
    VkDevice getDevice() { return m_device; }
    VkPhysicalDevice getPhysicalDevice() { return m_physicalDevice; }
    VkSurfaceKHR getSurface() { return m_surface; }
    VkQueue getGraphicsQueue() { return m_graphicsQueue; }
    VkQueue getPresentQueue() { return m_presentQueue; }
    VkSwapchainKHR getSwapChain() { return m_swapChain; }
    VkImageView getImageView(u32 index) { return m_imageViews[index]; }
    VkExtent2D getSwapChainExtent() { return m_swapChainExtent; }
    VkFormat getSwapChainImageFormat() { return m_swapChainImageFormat; }
    VkRenderPass getRenderPass() { return m_renderPass; }
    std::vector<VkFramebuffer> getFramebuffers() { return m_framebuffers; }
    VkCommandPool getCommandPool() { return m_commandPool; }
    VkCommandBuffer getCommandBuffer() {
        return m_commandBuffers[m_imageIndex];
    }
    VkCommandBuffer getCommandBuffer(u32 index) {
        return m_commandBuffers[index];
    }
    VmaAllocator getAllocator() { return m_allocator; }

    // utils
    static void check(VkResult res, const std::string &msg);

private:
    // struct
    struct QueueFamilyIndices
    {
        std::optional<u32> graphicsFamily;
        std::optional<u32> presentFamily;

        bool isComplete()
        {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    // Window
    core::Window *m_window;

    // Vulkan
    VkInstance m_instance;
    VkDebugUtilsMessengerEXT m_debug;
    VkSurfaceKHR m_surface;
    VkPhysicalDevice m_physicalDevice;
    QueueFamilyIndices m_queueFamilyIndices;
    VkDevice m_device;
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;
    VmaAllocator m_allocator;
    VkSwapchainKHR m_swapChain;
    std::vector<VkImage> m_images;
    VkFormat m_swapChainImageFormat;
    VkExtent2D m_swapChainExtent;
    std::vector<VkImageView> m_imageViews;
    VkImage m_depthImage;
    VmaAllocation m_depthImageAllocation;
    VkImageView m_depthImageView;
    VkFormat m_depthFormat;
    VkRenderPass m_renderPass;
    std::vector<VkFramebuffer> m_framebuffers;
    VkCommandPool m_commandPool;
    std::vector<VkCommandBuffer> m_commandBuffers;
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;

    // build
    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createAllocator();
    void createSwapChain();
    void createImageViews();
    void createDepthResources();
    void createRenderPass();
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects();

    // instance
    std::vector<const char *> getRequiredExtensions();
    bool checkValidationLayerSupport();
    void populateDebugMessengerCreateInfo(
        VkDebugUtilsMessengerCreateInfoEXT &createInfo
    );

    // debug
    VkResult CreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
        const VkAllocationCallbacks *pAllocator,
        VkDebugUtilsMessengerEXT *pDebugMessenger
    );

    void DestroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks *pAllocator
    );

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData
    );

    // physical device
    u32 rateDeviceSuitability(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    // swap chain
    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR> &availableFormats
    );
    VkPresentModeKHR chooseSwapPresentMode(
        const std::vector<VkPresentModeKHR> &availablePresentModes
    );
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

    // depth
    VkFormat findDepthFormat();
    VkFormat findSupportedFormat(
        const std::vector<VkFormat> &candidates,
        VkImageTiling tiling,
        VkFormatFeatureFlags features
    );
    
    // allocator
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    // variables
    const u32 FRAME_MAX_FRAMES_IN_FLIGHT = 2;
    u32 m_currentFrame = 0;
    u32 m_imageIndex = 0;

    // utils
    void recreateSwapChain();
    void cleanupSwapChain();
    bool hasStencilComponent(VkFormat format);
};

} // namespace gfx