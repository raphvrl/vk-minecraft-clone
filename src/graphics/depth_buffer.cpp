#include "depth_buffer.hpp"
#include "image.hpp"
#include "device.hpp"

namespace gfx
{

void DepthBuffer::init(Device &device, u32 width, u32 height)
{
    m_device = &device;
    m_format = findDepthFormat();

    m_image = m_device->createImage(
        width,
        height,
        m_format,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_IMAGE_ASPECT_DEPTH_BIT | (
            hasStencilComponent(m_format) ? VK_IMAGE_ASPECT_STENCIL_BIT : 0
        ),
        1,
        VK_SAMPLE_COUNT_1_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY
    );
}

void DepthBuffer::destroy()
{
    m_image.destroy();
}

void DepthBuffer::resize(u32 width, u32 height)
{
    m_image.destroy();
    m_image = m_device->createImage(
        width,
        height,
        m_format,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_IMAGE_ASPECT_DEPTH_BIT | (
            hasStencilComponent(m_format) ? VK_IMAGE_ASPECT_STENCIL_BIT : 0
        ),
        1,
        VK_SAMPLE_COUNT_1_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY
    );
}

VkFormat DepthBuffer::findDepthFormat()
{
    std::vector<VkFormat> candidates = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT
    };

    for (VkFormat format : candidates) {
        VkFormatProperties props = {};
        vkGetPhysicalDeviceFormatProperties(
            m_device->getPhysicalDevice(),
            format,
            &props
        );

        if (
            props.optimalTilingFeatures & 
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        ) {
            return format;
        }
    }

    throw std::runtime_error("Failed to find supported depth format!");
}

bool DepthBuffer::hasStencilComponent(VkFormat format) const
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
           format == VK_FORMAT_D24_UNORM_S8_UINT;
}

} // namespace gfx