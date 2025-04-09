#include "device.hpp"

namespace gfx
{

void Device::init(
    core::Window &window,
    const std::string &appName,
    const vk::Version &version
)
{
    m_window = &window;
    
    m_instance = vk::createInstance(appName, version);
    m_debugMessenger = vk::createDebugMessenger(m_instance);

    m_surface = vk::createSurface(window.get(), m_instance);
    
    m_physicalDevice = vk::pickPhysicalDevice(m_instance, m_surface);
    m_device = vk::createLogicalDevice(m_physicalDevice, m_surface);

    m_queueFamilyIndices = vk::findQueueFamilies(m_physicalDevice, m_surface);
    m_graphicsQueue = vk::getGraphicsQueue(m_device, m_queueFamilyIndices);
    m_presentQueue = vk::getPresentQueue(m_device, m_queueFamilyIndices);

    u32 width = m_window->getWidth();
    u32 height = m_window->getHeight();

    m_swapchain.init(m_device, m_physicalDevice, m_surface, width, height);

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        auto &frame = m_frames[i];
        frame.commandPool = vk::createCommandPool(
            m_device,
            m_queueFamilyIndices.graphicsFamily.value(),
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
        );
        frame.commandBuffer = vk::createCommandBuffer(
            m_device,
            frame.commandPool
        );
    }

    m_allocator = vk::createAllocator(m_instance, m_device, m_physicalDevice);
    m_depthBuffer.init(*this, width, height);

    m_defaultSampler = createSampler(
        VK_FILTER_NEAREST,
        VK_FILTER_NEAREST,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER
    );

    m_bindlessManager.init(*this);
}

void Device::destroy()
{
    m_bindlessManager.destroy();
    vkDestroySampler(m_device, m_defaultSampler, nullptr);

    m_depthBuffer.destroy();
    vmaDestroyAllocator(m_allocator);

    for (auto frame : m_frames) {
        vkDestroyCommandPool(m_device, frame.commandPool, nullptr);
    }

    m_swapchain.destroy();

    vkDestroyDevice(m_device, nullptr);
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);

    vk::destroyDebugMessenger(m_instance, m_debugMessenger);
    vkDestroyInstance(m_instance, nullptr);
}

VkCommandBuffer Device::beginFrame()
{
    m_swapchain.beginFrame(m_currentFrame);

    auto [imageIndex, image] = m_swapchain.acquireNextImage(m_currentFrame);
    m_imageIndex = imageIndex;

    if (m_swapchain.isOutOfDate()) {
        recreateSwapchain();
        return VK_NULL_HANDLE;
    }

    auto &frame = m_frames[m_currentFrame];
    
    vkResetCommandBuffer(frame.commandBuffer, 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VkResult res = vkBeginCommandBuffer(frame.commandBuffer, &beginInfo);
    vk::check(res, "Failed to begin command buffer");

    VkImageMemoryBarrier2 imageBarrierToRenderTarget = {};
    imageBarrierToRenderTarget.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    imageBarrierToRenderTarget.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
    imageBarrierToRenderTarget.srcAccessMask = 0;
    imageBarrierToRenderTarget.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    imageBarrierToRenderTarget.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    imageBarrierToRenderTarget.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageBarrierToRenderTarget.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    imageBarrierToRenderTarget.image = image;
    imageBarrierToRenderTarget.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageBarrierToRenderTarget.subresourceRange.baseMipLevel = 0;
    imageBarrierToRenderTarget.subresourceRange.levelCount = 1;
    imageBarrierToRenderTarget.subresourceRange.baseArrayLayer = 0;
    imageBarrierToRenderTarget.subresourceRange.layerCount = 1;

    VkRenderingAttachmentInfoKHR depthAttachment = {};
    depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
    depthAttachment.imageView = m_depthBuffer.getImageView();
    depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.clearValue.depthStencil = {1.0f, 0};

    VkImageMemoryBarrier2 depthBarrier = {};
    depthBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    depthBarrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
    depthBarrier.srcAccessMask = 0;
    depthBarrier.dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | 
                                VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
    depthBarrier.dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | 
                                VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    depthBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthBarrier.image = m_depthBuffer.getImage();
    depthBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    depthBarrier.subresourceRange.baseMipLevel = 0;
    depthBarrier.subresourceRange.levelCount = 1;
    depthBarrier.subresourceRange.baseArrayLayer = 0;
    depthBarrier.subresourceRange.layerCount = 1;

    VkDependencyInfoKHR dependencyInfo = {};
    dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    std::array<VkImageMemoryBarrier2, 2> barriers = {
        imageBarrierToRenderTarget, 
        depthBarrier
    };

    dependencyInfo.imageMemoryBarrierCount = static_cast<u32>(barriers.size());
    dependencyInfo.pImageMemoryBarriers = barriers.data();

    vkCmdPipelineBarrier2(frame.commandBuffer, &dependencyInfo);

    VkRenderingAttachmentInfoKHR colorAttachment = {};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
    colorAttachment.imageView = m_swapchain.getImageView(imageIndex);
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.clearValue.color = {{0.0f, 0.0f, 0.0f, 1.0f}};

    VkRenderingInfoKHR renderingInfo = {};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
    renderingInfo.renderArea.offset = {0, 0};
    renderingInfo.renderArea.extent = m_swapchain.getExtent();
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;
    renderingInfo.pDepthAttachment = &depthAttachment;

    vkCmdBeginRendering(frame.commandBuffer, &renderingInfo);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_swapchain.getExtent().width);
    viewport.height = static_cast<float>(m_swapchain.getExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_swapchain.getExtent();
    
    vkCmdSetViewport(frame.commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(frame.commandBuffer, 0, 1, &scissor);

    return frame.commandBuffer;
}

void Device::endFrame()
{
    if (m_swapchain.isOutOfDate()) {
        return;
    }

    auto &frame = m_frames[m_currentFrame];

    vkCmdEndRendering(frame.commandBuffer);

    VkImageMemoryBarrier2 imageBarrierToPresent{};
    imageBarrierToPresent.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    imageBarrierToPresent.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    imageBarrierToPresent.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    imageBarrierToPresent.dstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
    imageBarrierToPresent.dstAccessMask = 0;
    imageBarrierToPresent.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    imageBarrierToPresent.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    imageBarrierToPresent.image = m_swapchain.getImage(m_imageIndex);
    imageBarrierToPresent.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageBarrierToPresent.subresourceRange.baseMipLevel = 0;
    imageBarrierToPresent.subresourceRange.levelCount = 1;
    imageBarrierToPresent.subresourceRange.baseArrayLayer = 0;
    imageBarrierToPresent.subresourceRange.layerCount = 1;
    
    VkDependencyInfoKHR dependencyInfo{};
    dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependencyInfo.imageMemoryBarrierCount = 1;
    dependencyInfo.pImageMemoryBarriers = &imageBarrierToPresent;
    
    vkCmdPipelineBarrier2(frame.commandBuffer, &dependencyInfo);

    VkResult res = vkEndCommandBuffer(frame.commandBuffer);
    vk::check(res, "Failed to end command buffer");

    m_swapchain.submit(m_currentFrame, frame.commandBuffer, m_graphicsQueue);

    m_swapchain.present(m_currentFrame, m_presentQueue);
    if (m_swapchain.isOutOfDate()) {
        recreateSwapchain();
        return;
    }

    m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

VkCommandBuffer Device::beginSingleTimeCommands()
{
    auto &frame = m_frames[m_currentFrame];

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = frame.commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    VkResult res = vkAllocateCommandBuffers(
        m_device,
        &allocInfo,
        &commandBuffer
    );

    vk::check(res, "Failed to allocate command buffer for single time use");

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    res = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    vk::check(res, "Failed to begin single time command buffer");

    return commandBuffer;
}

void Device::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    VkResult res = vkEndCommandBuffer(commandBuffer);
    vk::check(res, "Failed to end single time command buffer");

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    res = vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vk::check(res, "Failed to submit single time command buffer");
    vkQueueWaitIdle(m_graphicsQueue);

    vkFreeCommandBuffers(
        m_device,
        m_frames[m_currentFrame].commandPool,
        1,
        &commandBuffer
    );
}

VkSampler Device::createSampler(
    VkFilter magFilter,
    VkFilter minFilter,
    VkSamplerAddressMode addressMode,
    f32 maxAnisotropy
)
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = magFilter;
    samplerInfo.minFilter = minFilter;
    samplerInfo.addressModeU = addressMode;
    samplerInfo.addressModeV = addressMode;
    samplerInfo.addressModeW = addressMode;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = maxAnisotropy > 0.0f ? 
        maxAnisotropy : properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    VkSampler sampler;
    VkResult res = vkCreateSampler(m_device, &samplerInfo, nullptr, &sampler);
    
    vk::check(res, "Failed to create texture sampler");

    return sampler;
}

Buffer Device::createBuffer(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VmaMemoryUsage memoryUsage
)
{
    Buffer buffer;
    buffer.init(*this, size, usage, memoryUsage);
    return buffer;
}

Image Device::createImage(
    u32 width,
    u32 height,
    VkFormat format,
    VkImageUsageFlags usage,
    VkImageAspectFlags aspectFlags,
    u32 mipLevels,
    VkSampleCountFlagBits samples,
    VmaMemoryUsage memoryUsage,
    bool createView
)
{
    Image image;
    image.init(
        *this,
        width,
        height,
        format,
        usage,
        aspectFlags,
        mipLevels,
        samples,
        memoryUsage,
        createView
    );

    return image;
}

Image Device::loadImage(
    const std::string &filepath,
    VkFormat format,
    VkImageUsageFlags additionalUsage,
    bool mipmaps,
    VkImageAspectFlags aspectFlags
)
{
    Image image;
    image.init(*this, filepath, format, additionalUsage, mipmaps, aspectFlags);
    return image;
}

Image Device::createImageFromData(
    const void *data,
    u32 width,
    u32 height,
    VkFormat format,
    VkImageUsageFlags additionalUsage,
    bool mipmaps,
    VkImageAspectFlags aspectFlags
)
{
    Image image;
    image.init(
        *this,
        data,
        width,
        height,
        format,
        additionalUsage,
        mipmaps,
        aspectFlags
    );

    return image;
}

void Device::waitIdle()
{
    vkDeviceWaitIdle(m_device);
}

void Device::recreateSwapchain()
{
    waitIdle();

    u32 width = m_window->getWidth();
    u32 height = m_window->getHeight();

    if (width == 0 || height == 0) {
        glfwWaitEvents();
        return;
    }

    m_swapchain.recreate(width, height);
    m_depthBuffer.resize(width, height);
}

} // namespace gfx