#include "texture_cache.hpp"

namespace gfx
{

void TextureCache::init(Device &device)
{
    m_device = &device;

    createDescriptorResources();

    load();
}

void TextureCache::destroy()
{
    for (auto &pair : m_textures) {
        pair.second.gpuImage.destroy();
        vkFreeDescriptorSets(
            m_device->getDevice(),
            m_descriptorPool,
            1,
            &pair.second.descriptorSet
        );
    }

    vkDestroyDescriptorPool(
        m_device->getDevice(),
        m_descriptorPool,
        nullptr
    );

    vkDestroyDescriptorSetLayout(
        m_device->getDevice(),
        m_descriptorSetLayout,
        nullptr
    );
}

void TextureCache::loadTexture(const fs::path &path, const std::string &name)
{
    if (m_textures.find(name) != m_textures.end()) {
        return;
    }

    VkFormat format = m_device->getSwapchain().getFormat();

    Image image = m_device->loadImage(
        path,
        format,
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        false
    );

    if (!image.isValid()) {
        throw std::runtime_error("Failed to load texture: " + path.string());
    }
    
    u32 textureID = m_device->addTexture(image);
    if (textureID == ~0u) {
        throw std::runtime_error("Failed to add texture to bindless manager.");
    }

    Texture texture;
    texture.path = path.string();
    texture.gpuImage = image;
    texture.index = textureID;
    texture.width = image.getWidth();
    texture.height = image.getHeight();
    texture.descriptorSet = allocateDescriptorSet();

    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = texture.gpuImage.getImageView();
    imageInfo.sampler = m_device->getDefaultSampler();

    VkWriteDescriptorSet writeSet = {};
    writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeSet.dstSet = texture.descriptorSet;
    writeSet.dstBinding = 0;
    writeSet.descriptorCount = 1;
    writeSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeSet.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(
        m_device->getDevice(),
        1,
        &writeSet,
        0,
        nullptr
    );

    m_textures[name] = texture;
}

void TextureCache::save()
{
    std::ofstream file(TEXTURE_CONFIG_PATH);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open textures.json for writing.");
    }

    json root = toJson();
    file << root.dump(4);
    file.close();
}

void TextureCache::load()
{
    std::ifstream file(TEXTURE_CONFIG_PATH);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open textures.json for reading.");
    }

    json root;
    file >> root;
    fromJson(root);
    file.close();
}

void TextureCache::createDescriptorResources()
{
    VkDescriptorSetLayoutBinding binding = {};
    binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding.descriptorCount = 1;
    binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    binding.binding = 0;

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &binding;

    VkResult res = vkCreateDescriptorSetLayout(
        m_device->getDevice(),
        &layoutInfo,
        nullptr,
        &m_descriptorSetLayout
    );

    vk::check(res, "Failed to create descriptor set layout.");

    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 256;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.maxSets = 256;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    res = vkCreateDescriptorPool(
        m_device->getDevice(),
        &poolInfo,
        nullptr,
        &m_descriptorPool
    );

    vk::check(res, "Failed to create descriptor pool.");
}

VkDescriptorSet TextureCache::allocateDescriptorSet()
{
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_descriptorSetLayout;

    VkDescriptorSet descriptorSet;
    VkResult res = vkAllocateDescriptorSets(
        m_device->getDevice(),
        &allocInfo,
        &descriptorSet
    );

    vk::check(res, "Failed to allocate descriptor set.");

    return descriptorSet;
}

json &TextureCache::toJson()
{
    json root;
    json texturesJson = json::object();

    for (const auto &[name, texture] : m_textures) {
        texturesJson[name] = {
            {"path", texture.path},
            {"name", name}
        };
    }

    root["textures"] = texturesJson;
    return root;
}

void TextureCache::fromJson(const json &root)
{
    if (root.contains("textures") && root["textures"].is_array()) {
        for (const auto &textureJson : root["textures"]) {
            std::string name = textureJson["name"];
            std::string path = textureJson["path"];

            loadTexture(path, name);
        }
    } else {
        throw std::runtime_error("Invalid texture config format.");
    }
}

} // namespace gfx