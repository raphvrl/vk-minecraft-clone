#pragma once

#include <stb_image.h>

#include "vulkan_ctx.hpp"

namespace gfx
{

static constexpr std::string_view TEXTURE_PATH = "assets/textures/";

class Texture
{

public:
    Texture() = default;
    virtual ~Texture() = default;

    Texture(const Texture &) = delete;
    Texture &operator=(const Texture &) = delete;

    void init(VulkanCtx &ctx, const std::string &path);
    void destroy();

    VkImageView getImageView() const { return m_imageView; }
    VkSampler getSampler() const { return m_sampler; }

private:
    VulkanCtx *m_ctx = nullptr;

    VkImage m_image;
    VmaAllocation m_imageAllocation;
    VkImageView m_imageView;
    VkSampler m_sampler;

    u32 m_width;
    u32 m_height;
    u32 m_mipLevels;

    void createImage(const std::string &path);
    void createImageView();
    void createSampler();

    void generateMipmaps();
};

} // namespace gfx