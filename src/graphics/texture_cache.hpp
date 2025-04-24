#pragma once

#include <nlohmann/json.hpp>

#include <fstream>

#include "device.hpp"
#include "image.hpp"

using json = nlohmann::json;

namespace gfx
{

struct Texture
{
    std::string path;

    Image gpuImage;
    u32 index;

    VkDescriptorSet descriptorSet;

    u32 width = 0;
    u32 height = 0;
};

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
            return it->second.index;
        }
        return ~0u;
    }

    VkDescriptorSet getDescriptorSet() const
    {
        return m_device->getDescriptorSet();
    }

    Texture *getTexture(const std::string &name)
    {
        auto it = m_textures.find(name);
        if (it != m_textures.end()) {
            return &it->second;
        }
        return nullptr;
    }

    std::vector<std::string> getTextureNames() const
    {
        std::vector<std::string> names;
        for (const auto &pair : m_textures) {
            names.push_back(pair.first);
        }
        return names;
    }

    void load();
    void save();

private:
    Device *m_device = nullptr;

    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;

    void createDescriptorResources();
    VkDescriptorSet allocateDescriptorSet();

    std::unordered_map<std::string, Texture> m_textures;

    const char *TEXTURE_CONFIG_PATH = "assets/config/textures.json";

    json &toJson();
    void fromJson(const json &data);
}; 

} // namespace gfx