#include "swapchain.hpp"

namespace gfx
{

void Swapchain::init(
    VkDevice device,
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    u32 width,
    u32 height
)
{
    m_device = device;
    m_physicalDevice = physicalDevice;
    m_surface = surface;

    m_swapchain = vk::createSwapchain(
        device,
        physicalDevice,
        surface,
        width,
        height,
        VK_FORMAT_B8G8R8A8_UNORM,
        VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        VK_PRESENT_MODE_MAILBOX_KHR,
        0,
        &m_imageFormat,
        &m_extent
    );

    m_images = vk::getSwapchainImages(device, m_swapchain);

    m_imageViews = vk::createSwapchainImageViews(
        device,
        m_images,
        m_imageFormat
    );

    m_frames.resize(MAX_FRAMES_IN_FLIGHT);

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        m_frames[i].imageSemaphore = vk::createSemaphore(device);
        m_frames[i].renderSemaphore = vk::createSemaphore(device);
        m_frames[i].renderFence = vk::createFence(
            device,
            VK_FENCE_CREATE_SIGNALED_BIT
        );
    }
}

void Swapchain::destroy()
{
    for (auto frame : m_frames) {
        vkDestroySemaphore(m_device, frame.imageSemaphore, nullptr);
        vkDestroySemaphore(m_device, frame.renderSemaphore, nullptr);
        vkDestroyFence(m_device, frame.renderFence, nullptr);
    }

    for (auto imageView : m_imageViews) {
        vkDestroyImageView(m_device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    
    m_images.clear();
    m_imageViews.clear();
}

void Swapchain::recreate(u32 width, u32 height)
{
    vkDeviceWaitIdle(m_device);
    
    cleanupSwapchain();
    
    m_swapchain = vk::createSwapchain(
        m_device,
        m_physicalDevice,
        m_surface,
        width,
        height,
        m_imageFormat,                 
        VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        VK_PRESENT_MODE_MAILBOX_KHR,
        0,                              
        &m_imageFormat,                 
        &m_extent,                      
        VK_NULL_HANDLE                  
    );
    
    m_images = vk::getSwapchainImages(m_device, m_swapchain);
    
    m_imageViews = vk::createSwapchainImageViews(
        m_device,
        m_images,
        m_imageFormat
    );

    m_outOfDate = false;
}

void Swapchain::beginFrame(u32 &currentFrame)
{
    auto &frame = m_frames[currentFrame];

    VkResult res = vkWaitForFences(
        m_device,
        1,
        &frame.renderFence,
        VK_TRUE,
        U64_MAX
    );

    vk::check(res, "Failed to wait for fence.");
}

std::pair<u32, VkImage> Swapchain::acquireNextImage(u32 currentFrame)
{
    auto &frame = m_frames[currentFrame];

    VkResult res = vkResetFences(m_device, 1, &frame.renderFence);
    vk::check(res, "Failed to reset fence.");

    res = vkAcquireNextImageKHR(
        m_device,
        m_swapchain,
        U64_MAX,
        frame.imageSemaphore,
        VK_NULL_HANDLE,
        &m_imageIndex
    );

    if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) {
        m_outOfDate = true;
        return {0, VK_NULL_HANDLE};
    } else if (res != VK_SUCCESS) {
        vk::check(res, "Failed to acquire swapchain image.");
    }

    return {m_imageIndex, m_images[m_imageIndex]};
}

void Swapchain::submit(
    u32 currentFrame,
    VkCommandBuffer commandBuffer,
    VkQueue graphicsQueue
)
{
    if (m_outOfDate) {
        return;
    }

    auto &frame = m_frames[currentFrame];

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {frame.imageSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkSemaphore signalSemaphores[] = {frame.renderSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    VkResult res = vkQueueSubmit(
        graphicsQueue,
        1,
        &submitInfo,
        frame.renderFence
    );

    vk::check(res, "Failed to submit draw command buffer.");
}

void Swapchain::present(u32 currentFrame, VkQueue presentQueue)
{
    if (m_outOfDate) {
        return;
    }
    
    auto &frame = m_frames[currentFrame];
    
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    
    VkSemaphore signalSemaphores[] = {frame.renderSemaphore};
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    
    VkSwapchainKHR swapchains[] = {m_swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &m_imageIndex;
    
    presentInfo.pResults = nullptr;
    
    VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        m_outOfDate = true;
    } else if (result != VK_SUCCESS) {
        vk::check(result, "Failed to present swap chain image.");
    }
}

void Swapchain::cleanupSwapchain()
{
    for (auto imageView : m_imageViews) {
        vkDestroyImageView(m_device, imageView, nullptr);
    }
    
    if (m_swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
        m_swapchain = VK_NULL_HANDLE;
    }
    
    m_images.clear();
    m_imageViews.clear();
} 

} // namespace gfx