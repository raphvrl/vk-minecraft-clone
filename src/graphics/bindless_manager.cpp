#include "bindless_manager.hpp"
#include "device.hpp"
#include "buffer.hpp"
#include "image.hpp"

namespace gfx
{

void BindlessManager::init(Device &device)
{
    m_device = &device;

    VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexingFeatures{};
    indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;

    VkPhysicalDeviceFeatures2 features2{};
    features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    features2.pNext = &indexingFeatures;

    vkGetPhysicalDeviceFeatures2(device.getPhysicalDevice(), &features2);

    if (
        !indexingFeatures.descriptorBindingPartiallyBound ||
        !indexingFeatures.runtimeDescriptorArray
    ) {
        throw std::runtime_error("Descriptor indexing features not supported.");
    }

    std::vector<VkDescriptorPoolSize> poolSizes = {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_UBOS},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_SSBOS},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES}
    };

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT;
    poolInfo.maxSets = 1;
    poolInfo.poolSizeCount = static_cast<u32>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();

    VkResult res = vkCreateDescriptorPool(
        device.getDevice(),
        &poolInfo,
        nullptr,
        &m_descriptorPool
    );

    vk::check(res, "Failed to create descriptor pool.");

    std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {
            UBO_BINDING,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            MAX_UBOS,
            VK_SHADER_STAGE_ALL,
            nullptr
        },
        {
            SSBO_BINDING,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            MAX_SSBOS,
            VK_SHADER_STAGE_ALL,
            nullptr
        },
        {
            TEXTURE_BINDING,
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            MAX_TEXTURES,
            VK_SHADER_STAGE_ALL,
            nullptr
        }
    };

    std::vector<VkDescriptorBindingFlagsEXT> bindingFlags = {
        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT |
        VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT,

        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT |
        VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT,

        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT |
        VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT
    };

    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT bindingFlagsInfo{};
    bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
    bindingFlagsInfo.bindingCount = static_cast<u32>(bindingFlags.size());
    bindingFlagsInfo.pBindingFlags = bindingFlags.data();

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
    layoutInfo.bindingCount = static_cast<u32>(bindings.size());
    layoutInfo.pBindings = bindings.data();
    layoutInfo.pNext = &bindingFlagsInfo;

    res = vkCreateDescriptorSetLayout(
        device.getDevice(),
        &layoutInfo,
        nullptr,
        &m_descriptorSetLayout
    );

    vk::check(res, "Failed to create descriptor set layout.");

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_descriptorSetLayout;

    res = vkAllocateDescriptorSets(
        device.getDevice(),
        &allocInfo,
        &m_descriptorSet
    );

    vk::check(res, "Failed to allocate descriptor set.");

    m_resources.resize(MAX_UBOS + MAX_SSBOS + MAX_TEXTURES);
    m_dirtyResources.resize(MAX_UBOS + MAX_SSBOS + MAX_TEXTURES);
}

void BindlessManager::destroy()
{
    vkDestroyDescriptorSetLayout(
        m_device->getDevice(),
        m_descriptorSetLayout,
        nullptr
    );

    vkDestroyDescriptorPool(
        m_device->getDevice(),
        m_descriptorPool,
        nullptr
    );

    m_resources.clear();
    m_dirtyResources.clear();
}

u32 BindlessManager::addUBO(
    const Buffer &buffer,
    VkDeviceSize offset,
    VkDeviceSize range
)
{
    if (range == 0) {
        range = buffer.getSize() - offset;
    }

    u32 handle = addResourceInternal(
        ResourceType::UBO,
        UBO_BINDING,
        m_nextUboIndex,
        MAX_UBOS
    );

    if (handle != ~0u) {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_resources[handle].buffer = buffer.getBuffer();
        m_resources[handle].offset = offset;
        m_resources[handle].range = range;
        m_resources[handle].isDirty = true;

        m_dirtyResources.push_back(handle);
    }

    return handle;
}

u32 BindlessManager::addSSBO(
    const Buffer &buffer,
    VkDeviceSize offset,
    VkDeviceSize range
)
{
    if (range == 0) {
        range = buffer.getSize() - offset;
    }

    u32 handle = addResourceInternal(
        ResourceType::SSBO,
        SSBO_BINDING,
        m_nextSsboIndex,
        MAX_SSBOS
    );

    if (handle != ~0u) {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_resources[handle].buffer = buffer.getBuffer();
        m_resources[handle].offset = offset;
        m_resources[handle].range = range;
        m_resources[handle].isDirty = true;

        m_dirtyResources.push_back(handle);
    }

    return handle;
}

u32 BindlessManager::addTexture(
    const Image &image,
    VkSampler sampler
)
{
    VkImageView imageView = image.getImageView();
    if (imageView == VK_NULL_HANDLE) {
        throw std::runtime_error("Image view is not created.");
    }

    if (sampler == VK_NULL_HANDLE) {
        sampler = m_device->getDefaultSampler();
    }

    u32 handle = addResourceInternal(
        ResourceType::TEXTURE,
        TEXTURE_BINDING,
        m_nextTextureIndex,
        MAX_TEXTURES
    );

    if (handle != ~0u) {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_resources[handle].imageView = image.getImageView();
        m_resources[handle].sampler = sampler;
        m_resources[handle].isDirty = true;

        m_dirtyResources.push_back(handle);
    }

    return handle;
}

void BindlessManager::removeResource(u32 id)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (id >= m_resources.size() || !m_resources[id].isUsed) {
        return;
    }

    m_resources[id] = ResourceSlot{};
}

void BindlessManager::update()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_dirtyResources.empty()) {
        return;
    }

    std::vector<VkWriteDescriptorSet> writeSets;
    std::vector<VkDescriptorBufferInfo> bufferInfos;
    std::vector<VkDescriptorImageInfo> imageInfos;

    for (u32 index : m_dirtyResources) {
        auto &resource = m_resources[index];

        if (!resource.isUsed || !resource.isDirty) {
            continue;
        }

        bufferInfos.push_back({
            resource.buffer,
            resource.offset,
            resource.range
        });

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = m_descriptorSet;
        write.dstBinding = resource.binding;
        write.dstArrayElement = resource.arrayIndex;
        write.descriptorCount = 1;

        switch (resource.type) {
            case ResourceType::UBO:
                write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                bufferInfos.push_back({
                    resource.buffer,
                    resource.offset,
                    resource.range
                });
                write.pBufferInfo = &bufferInfos.back();
                break;
            
            case ResourceType::SSBO:
                write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                bufferInfos.push_back({
                    resource.buffer,
                    resource.offset,
                    resource.range
                });
                write.pBufferInfo = &bufferInfos.back();
                break;
            
            case ResourceType::TEXTURE:
                write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                imageInfos.push_back({
                    resource.sampler,
                    resource.imageView,
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                });
                write.pImageInfo = &imageInfos.back();
                break;
            default:
                continue;
        }

        writeSets.push_back(write);
        resource.isDirty = false;
    }

    if (!writeSets.empty()) {
        vkUpdateDescriptorSets(
            m_device->getDevice(),
            static_cast<u32>(writeSets.size()),
            writeSets.data(),
            0,
            nullptr
        );
    }

    m_dirtyResources.clear();
}

u32 BindlessManager::addResourceInternal(
    ResourceType type,
    u32 binding,
    u32 &nextIndex,
    u32 maxCount
)
{
    if (nextIndex >= maxCount) {
        return ~0u;
    }

    u32 index = nextIndex;

    m_resources[index].type = type;
    m_resources[index].binding = binding;
    m_resources[index].arrayIndex = nextIndex;
    m_resources[index].isUsed = true;

    nextIndex++;

    return index;
}

} // namespace gfx