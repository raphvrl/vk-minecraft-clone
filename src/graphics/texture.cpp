#include "texture.hpp"

namespace gfx
{

void Texture::init(VulkanCtx &ctx, const std::string &path)
{
    m_ctx = &ctx;

    createImage(path);
    createImageView();
    createSampler();
}

void Texture::destroy()
{
    vkDeviceWaitIdle(m_ctx->getDevice());

    vkDestroySampler(m_ctx->getDevice(), m_sampler, nullptr);
    vkDestroyImageView(m_ctx->getDevice(), m_imageView, nullptr);
    vmaDestroyImage(m_ctx->getAllocator(), m_image, m_imageAllocation);
}

void Texture::createImage(const std::string &path)
{
    std::string fullPath = std::string(TEXTURE_PATH) + path;

    stbi_set_flip_vertically_on_load(true);

    int texWidth, texHeight, texChannels;
    stbi_uc *pixels = stbi_load(
        fullPath.c_str(),
        &texWidth,
        &texHeight,
        &texChannels,
        STBI_rgb_alpha
    );

    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    VkBuffer stagingBuffer;
    VmaAllocation stagingBufferAllocation;
    m_ctx->createBuffer(
        imageSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_MEMORY_USAGE_CPU_ONLY,
        stagingBuffer,
        stagingBufferAllocation
    );

    void *data;
    vmaMapMemory(m_ctx->getAllocator(), stagingBufferAllocation, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vmaUnmapMemory(m_ctx->getAllocator(), stagingBufferAllocation);

    stbi_image_free(pixels);

    m_mipLevels = static_cast<u32>(
        std::min(
            std::floor(std::log2(std::max(texWidth, texHeight))  + 1),
            std::floor(std::log2(std::max(texWidth, texHeight) / MIN_MIP_SIZE) + 1)
        )
    );

    m_ctx->createImage(
        texWidth,
        texHeight,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | 
        VK_IMAGE_USAGE_TRANSFER_DST_BIT |
        VK_IMAGE_USAGE_SAMPLED_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY,
        m_image,
        m_imageAllocation,
        m_mipLevels
    );

    m_ctx->transitionImageLayout(
        m_image,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        m_mipLevels
    );

    m_ctx->copyBufferToImage(
        stagingBuffer,
        m_image,
        static_cast<u32>(texWidth),
        static_cast<u32>(texHeight)
    );

    vmaDestroyBuffer(
        m_ctx->getAllocator(),
        stagingBuffer,
        stagingBufferAllocation
    );

    m_width = texWidth;
    m_height = texHeight;

    generateMipmaps();
}

void Texture::createImageView()
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = m_mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkResult res = vkCreateImageView(
        m_ctx->getDevice(),
        &viewInfo,
        nullptr,
        &m_imageView
    );

    m_ctx->check(res, "failed to create texture image view!");
}

void Texture::createSampler()
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_NEAREST;
    samplerInfo.minFilter = VK_FILTER_NEAREST;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = m_mipLevels;
    samplerInfo.mipLodBias = 0.0f;

    VkResult res = vkCreateSampler(
        m_ctx->getDevice(),
        &samplerInfo,
        nullptr,
        &m_sampler
    );

    m_ctx->check(res, "failed to create texture sampler!");
}

void Texture::generateMipmaps()
{
    VkCommandBuffer commandBuffer = m_ctx->beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = m_image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    i32 mipWidth = m_width;
    i32 mipHeight = m_height;

    for (uint32_t i = 1; i < m_mipLevels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0,
            nullptr,
            0,
            nullptr,
            1,
            &barrier
        );

        VkImageBlit blit{};
        blit.srcOffsets[0] = { 0, 0, 0 };
        blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = { 0, 0, 0 };
        blit.dstOffsets[1] = {
            mipWidth > 1 ? mipWidth / 2 : 1,
            mipHeight > 1 ? mipHeight / 2 : 1,
            1 
        };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        barrier.subresourceRange.baseMipLevel = i;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0,
            nullptr,
            0,
            nullptr,
            1,
            &barrier
        );

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
            0,
            nullptr,
            0,
            nullptr,
            1,
            &barrier
        );

        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = m_mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &barrier
    );

    m_ctx->endSingleTimeCommands(commandBuffer);
}

} // namespace gfx