#pragma once

#include "device.hpp"
#include "image.hpp"

constexpr const char* TEXTURE_DIR_STR = "assets/textures/";

namespace gfx
{

class TextureCache
{

public:
    TextureCache() = default;
    ~TextureCache() = default;

    void init(Device &device);
    void destroy();

    void loadTexture(const fs::path &path, const std::string &name);

    u32 getTextureID(const std::string &name) const
    {
        auto it = m_textures.find(name);
        if (it != m_textures.end()) {
            return it->second.second;
        }
        return ~0u;
    }

private:
    Device *m_device = nullptr;

    std::unordered_map<std::string, std::pair<Image, u32>> m_textures;

}; 

} // namespace gfx