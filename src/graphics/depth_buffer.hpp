#pragma once

#include <vulkan/vulkan.h>

#include "core/types.hpp"
#include "image.hpp"

namespace gfx
{

class Device;

class DepthBuffer
{

public:
    DepthBuffer() = default;
    ~DepthBuffer() = default;

    void init(Device &device, u32 width, u32 height);
    void destroy();
    void resize(u32 width, u32 height);

public:
    VkFormat getFormat() const { return m_format; }
    VkImage getImage() const { return m_image.getImage(); }
    VkImageView getImageView() const { return m_image.getImageView(); }

private:
    Device *m_device = nullptr;

    Image m_image;
    VkFormat m_format = VK_FORMAT_UNDEFINED;

private:
    VkFormat findDepthFormat();
    bool hasStencilComponent(VkFormat format) const;

};

} // namespace gfx