#pragma once

#include "device.hpp"
#include "image.hpp"

namespace gfx
{

class Framebuffer
{

public:
    Framebuffer() = default;
    ~Framebuffer() = default;

    void init(
        Device &device,
        u32 width,
        u32 height,
        VkFormat colorFormat = VK_FORMAT_R8G8B8A8_UNORM,
        VkFormat depthFormat = VK_FORMAT_D32_SFLOAT,
        bool withDepth = true
    );

    void destroy();
    void resize(u32 width, u32 height);

    void begin(VkCommandBuffer cmd);
    void end(VkCommandBuffer cmd);

public:
    u32 getWidth() const { return m_width; }
    u32 getHeight() const { return m_height; }

    Image &getColorImage() { return m_colorImage; }
    Image &getDepthImage() { return m_depthImage; }

    VkFormat getColorFormat() const { return m_colorFormat; }
    VkFormat getDepthFormat() const { return m_depthFormat; }

    u32 getTextureID() const { return m_textureID; }

private:
    Device *m_device = nullptr;

    Image m_colorImage;
    Image m_depthImage;

    u32 m_width = 0;
    u32 m_height = 0;
    VkFormat m_colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
    VkFormat m_depthFormat = VK_FORMAT_D32_SFLOAT;
    bool m_withDepth = true;

    u32 m_textureID = U32_MAX;

private:
    void transitionColorLayout(
        VkCommandBuffer cmd,
        VkImageLayout oldLayout,
        VkImageLayout newLayout
    );

};

} // namespace gfx