#include "image.hpp"
#include "device.hpp"
#include "stb_image.h"

namespace gfx
{

void Image::destroy()
{
    if (m_imageView) {
        vkDestroyImageView(m_device->getDevice(), m_imageView, nullptr);
    }

    if (m_image) {
        vmaDestroyImage(m_device->getAllocator(), m_image, m_allocation);
    }
}

VkImageView Image::createView(
    VkFormat format,
    VkImageAspectFlags aspectFlags,
    u32 baseMipLevel,
    u32 levelCount
)
{
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = baseMipLevel;
    viewInfo.subresourceRange.levelCount = levelCount;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(m_device->getDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create image view!");
    }

    return imageView;
}

void Image::transitionLayout(
    VkImageLayout oldLayout,
    VkImageLayout newLayout,
    VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask
)
{
    VkCommandBuffer commandBuffer = m_device->beginSingleTimeCommands();

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_image;
    barrier.subresourceRange.aspectMask = m_aspectFlags;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = m_mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    if (
        oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    ) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    } else if (
        oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    ) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    } else if (
        oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    ) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = 
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | 
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    } else {
        throw std::invalid_argument("Unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        srcStageMask,
        dstStageMask,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    m_device->endSingleTimeCommands(commandBuffer);

    m_layout = newLayout;
}

void Image::generateMipmaps()
{
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(
        m_device->getPhysicalDevice(),
        m_format,
        &formatProperties
    );

    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        throw std::runtime_error("Texture image format does not support linear blitting!");
    }

    VkCommandBuffer commandBuffer = m_device->beginSingleTimeCommands();

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = m_image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = m_aspectFlags;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int mipWidth = static_cast<int>(m_width);
    int mipHeight = static_cast<int>(m_height);

    barrier.subresourceRange.baseMipLevel = 0;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    for (u32 i = 1; i < m_mipLevels; i++) {
        barrier.subresourceRange.baseMipLevel = i;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        VkImageBlit blit = {};
        blit.srcOffsets[0] = { 0, 0, 0 };
        blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
        blit.srcSubresource.aspectMask = m_aspectFlags;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = { 0, 0, 0 };
        blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
        blit.dstSubresource.aspectMask = m_aspectFlags;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(
            commandBuffer,
            m_image,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            m_image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &blit,
            VK_FILTER_LINEAR
        );

        if (i > 1) {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(
                commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );
        }

        if (i == m_mipLevels - 1) {
            barrier.subresourceRange.baseMipLevel = i;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(
                commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );
        } else {
            barrier.subresourceRange.baseMipLevel = i;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(
                commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );
        }

        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = 0;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    m_device->endSingleTimeCommands(commandBuffer);
    m_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

void Image::copyFromBuffer(Buffer &buffer)
{
    VkCommandBuffer commandBuffer = m_device->beginSingleTimeCommands();

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = m_aspectFlags;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { m_width, m_height, 1 };

    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer.getBuffer(),
        m_image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    m_device->endSingleTimeCommands(commandBuffer);
}

void Image::init(
    Device &device,
    u32 width,
    u32 height,
    VkFormat format,
    VkImageUsageFlags usage,
    VkImageAspectFlags aspectFlags,
    u32 mipLevels,
    VkSampleCountFlagBits samples,
    VmaMemoryUsage memoryUsage,
    bool imageView
)
{
    m_device = &device;
    m_width = width;
    m_height = height;
    m_format = format;
    m_mipLevels = mipLevels;
    m_samples = samples;
    m_aspectFlags = aspectFlags;

    createImage(
        width,
        height,
        format,
        usage,
        mipLevels,
        samples,
        memoryUsage
    );

    if (imageView) {
        m_imageView = createView(format, aspectFlags);
    }
}

void Image::init(
    Device &device,
    const fs::path &filepath,
    VkFormat format,
    VkImageUsageFlags additionalUsage,
    bool mipmaps,
    VkImageAspectFlags aspectFlags
)
{
    m_device = &device;

    stbi_set_flip_vertically_on_load(true);

    int width, height, channels;
    stbi_uc *pixels = stbi_load(
        filepath.string().c_str(),
        &width,
        &height,
        &channels,
        STBI_rgb_alpha
    );

    if (!pixels) {
        throw std::runtime_error(
            "Failed to load image file: " + filepath.string()
        );
    }

    VkDeviceSize imageSize = width * height * 4;

    u32 mipLevels = 1;
    if (mipmaps) {
        u32 max = std::max(width, height);
        mipLevels = static_cast<u32>(std::floor(std::log2(max))) + 1;
    }

    Buffer stagingBuffer = m_device->createBuffer(
        imageSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU
    );

    stagingBuffer.uploadData(pixels, imageSize);

    stbi_image_free(pixels);

    VkImageUsageFlags usage = 
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    if (mipmaps) {
        usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    
    usage |= additionalUsage;

    init(
        device,
        width,
        height,
        format,
        usage,
        aspectFlags,
        mipLevels,
        VK_SAMPLE_COUNT_1_BIT,
        VMA_MEMORY_USAGE_AUTO,
        false
    );

    transitionLayout(
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT
    );

    copyFromBuffer(stagingBuffer);

    if (mipmaps) {
        generateMipmaps();
    } else {
        transitionLayout(
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
        );
    }

    m_imageView = createView(format, aspectFlags);
    stagingBuffer.destroy();
}

void Image::init(
    Device &device,
    const void *data,
    u32 width,
    u32 height,
    VkFormat format,
    VkImageUsageFlags additionalUsage,
    bool mipmaps,
    VkImageAspectFlags aspectFlags
)
{
    VkDeviceSize imageSize = width * height * 4;

    u32 mipLevels = 1;
    if (mipmaps) {
        u32 max = std::max(width, height);
        mipLevels = static_cast<u32>(std::floor(std::log2(max))) + 1;
    }

    Buffer stagingBuffer = m_device->createBuffer(
        imageSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU
    );

    void *mappedData = stagingBuffer.map();
    memcpy(mappedData, data, imageSize);
    stagingBuffer.unmap();

    VkImageUsageFlags usage = 
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    if (mipmaps) {
        usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    usage |= additionalUsage;

    init(
        device,
        width,
        height,
        format,
        usage,
        aspectFlags,
        mipLevels,
        VK_SAMPLE_COUNT_1_BIT,
        VMA_MEMORY_USAGE_AUTO,
        false
    );

    transitionLayout(
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT
    );

    copyFromBuffer(stagingBuffer);

    if (mipmaps) {
        generateMipmaps();
    } else {
        transitionLayout(
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
        );
    }

    createImageView(m_aspectFlags);

    stagingBuffer.destroy();
}

void Image::createImage(
    u32 width,
    u32 height,
    VkFormat format,
    VkImageUsageFlags usage,
    u32 mipLevels,
    VkSampleCountFlagBits samples,
    VmaMemoryUsage memoryUsage
)
{
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = samples;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = memoryUsage;

    if (vmaCreateImage(
        m_device->getAllocator(),
        &imageInfo,
        &allocInfo,
        &m_image,
        &m_allocation,
        nullptr
    ) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create image!");
    }
}

void Image::createImageView(VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = m_format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = m_mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkResult res = vkCreateImageView(
        m_device->getDevice(),
        &viewInfo,
        nullptr,
        &m_imageView
    );

    vk::check(res, "Failed to create image view!");
}

} // namespace gfx