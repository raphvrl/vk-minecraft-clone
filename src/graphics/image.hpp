#pragma once 

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <string>
#include <filesystem>

#include "core/types.hpp"

namespace fs = std::filesystem;

namespace gfx
{

class Device;
class Buffer;

class Image
{

public:
    Image() = default;
    ~Image() = default;

    void destroy();

    VkImageView createView(
        VkFormat format,
        VkImageAspectFlags aspectFlags,
        u32 baseMipLevel = 0,
        u32 levelCount = VK_REMAINING_MIP_LEVELS
    );

    void transitionLayout(
        VkImageLayout oldLayout,
        VkImageLayout newLayout,
        VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
    );

    void generateMipmaps();

    void copyFromBuffer(Buffer &buffer);

public:
    VkImage getImage() const { return m_image; }
    VkImageView getImageView() const { return m_imageView; }
    VmaAllocation getAllocation() const { return m_allocation; }

    VkFormat getFormat() const { return m_format; }
    VkImageLayout getLayout() const { return m_layout; }

    u32 getWidth() const { return m_width; }
    u32 getHeight() const { return m_height; }
    u32 getMipLevels() const { return m_mipLevels; }
    VkSampleCountFlagBits getSamples() const { return m_samples; }
    VkImageAspectFlags getAspectFlags() const { return m_aspectFlags; }

private:
    Device *m_device = nullptr;

    VkImage m_image = VK_NULL_HANDLE;
    VkImageView m_imageView = VK_NULL_HANDLE;
    VmaAllocation m_allocation = VK_NULL_HANDLE;

    VkFormat m_format = VK_FORMAT_UNDEFINED;
    VkImageLayout m_layout = VK_IMAGE_LAYOUT_UNDEFINED;

    u32 m_width = 0;
    u32 m_height = 0;
    u32 m_mipLevels = 1;
    VkSampleCountFlagBits m_samples = VK_SAMPLE_COUNT_1_BIT;
    VkImageAspectFlags m_aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;

private:
    friend class Device;

    void init(
        Device &device,
        u32 width,
        u32 height,
        VkFormat format,
        VkImageUsageFlags usage,
        VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT,
        u32 mipLevels = 1,
        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT,
        VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO,
        bool imageView = true
    );

    void init(
        Device &device,
        const fs::path &filepath,
        VkFormat format = VK_FORMAT_R8G8B8A8_SRGB,
        VkImageUsageFlags additionalUsage = 0,
        bool mipmaps = true,
        VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT
    );

    void init(
        Device &device,
        const void *data,
        u32 width,
        u32 height,
        VkFormat format = VK_FORMAT_R8G8B8A8_SRGB,
        VkImageUsageFlags additionalUsage = 0,
        bool mipmaps = true,
        VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT
    );

    void createImage(
        u32 width,
        u32 height,
        VkFormat format,
        VkImageUsageFlags usage,
        u32 mipLevels,
        VkSampleCountFlagBits samples,
        VmaMemoryUsage memoryUsage
    );

    void createImageView(VkImageAspectFlags aspectFlags);

};

} // namespace gfx