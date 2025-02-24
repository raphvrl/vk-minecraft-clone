#pragma once

#include <vector>
#include <string>
#include <string_view>
#include <fstream>
#include <stdexcept>

#include "vulkan_ctx.hpp"
#include "texture.hpp"

namespace gfx
{

static constexpr std::string_view SHADER_PATH = "assets/shaders/";

enum class ShaderType
{
    VERTEX,
    FRAGMENT,
    GEOMETRY
};

struct DescriptorLayout
{
    u32 binding;
    VkDescriptorType type;
    u32 count;
    VkShaderStageFlags stage;
};

struct UniformBuffer
{
    VkBuffer buffer;
    VmaAllocation allocation;
    VkDeviceSize size;
};

class Pipeline
{

public:
    class Builder
    {

    public:
        Builder(VulkanCtx &ctx);

        Builder &setShader(ShaderType type, const std::string &path);
        Builder &addPushConstant(ShaderType type, u32 offset, u32 size);
        Builder &setVertexInput(
            const VkVertexInputBindingDescription *bindingDescription,
            const VkVertexInputAttributeDescription *attributeDescriptions,
            u32 attributeCount  
        );
        Builder &addDescriptorBinding(const DescriptorLayout &layout);
        Builder &setDepthTest(bool enable);
        Builder &setDepthWrite(bool enable);

        Pipeline build();

    private:
        VulkanCtx& m_ctx;

        std::map<int, std::string> m_shaderPaths;
        std::vector<VkPushConstantRange> m_pushConstants;

        bool m_hasVertexInput = false;
        VkVertexInputBindingDescription m_bindingDescription = {};
        std::vector<VkVertexInputAttributeDescription> m_attributeDescriptions;

        std::vector<DescriptorLayout> m_descriptorLayouts;

        bool m_depthTest = true;
        bool m_depthWrite = true;

        std::vector<char> readFile(const std::string &path);
        VkShaderModule createShaderModule(const std::vector<char> &code);
    };

    Pipeline() = default;

    void destroy();

    VkDescriptorSet createDescriptorSet(Texture &texture);
    void bindDescriptorSet(VkDescriptorSet set);

    void bind();
    void push(ShaderType type, u32 offset, u32 size, const void *data);

private:
    friend class Builder;

    VulkanCtx *m_ctx;

    VkPipeline m_handle;
    VkPipelineLayout m_layout;
    VkDescriptorSetLayout m_descriptor;
    VkDescriptorPool m_descriptorPool;
};
    
} // namespace gfx