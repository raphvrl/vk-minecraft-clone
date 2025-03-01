#pragma once

#include <vector>
#include <string>
#include <string_view>
#include <fstream>
#include <stdexcept>

#include "vulkan_ctx.hpp"
#include "texture.hpp"
#include "uniform_buffer.hpp"

namespace gfx
{

static constexpr std::string_view SHADER_PATH = "assets/shaders/";

struct DescriptorLayout
{
    u32 binding;
    VkDescriptorType type;
    u32 count;
    VkShaderStageFlags stage;
};

struct DescriptorData
{
    VkDescriptorType type;
    u32 binding;
    VkShaderStageFlags stage;
    union {
        Texture *texture;
        UniformBuffer *ubo;
    };
};

class Pipeline
{

public:
    class Builder
    {

    public:
        Builder(VulkanCtx &ctx);

        Builder &setShader(VkShaderStageFlags stage, const std::string &path);
        Builder &addPushConstant(
            VkShaderStageFlags stage,
            u32 offset,
            u32 size
        );
        Builder &setVertexInput(
            const VkVertexInputBindingDescription *bindingDescription,
            const VkVertexInputAttributeDescription *attributeDescriptions,
            u32 attributeCount  
        );
        Builder &addDescriptorBinding(const DescriptorLayout &layout);
        Builder &setDepthTest(bool enable);
        Builder &setDepthWrite(bool enable);
        Builder &setCullMode(VkCullModeFlags mode);
        Builder &setTopology(VkPrimitiveTopology topology);
        Builder &setLineWidth(f32 width);
        Builder &setBlending(bool enable);
        Builder &setPolygonMode(VkPolygonMode mode);

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

        VkCullModeFlags m_cullMode = VK_CULL_MODE_BACK_BIT;

        VkPrimitiveTopology m_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        f32 m_lineWidth = 1.0f;

        bool m_blending = false;

        VkPolygonMode m_polygonMode = VK_POLYGON_MODE_FILL;

        std::vector<char> readFile(const std::string &path);
        VkShaderModule createShaderModule(const std::vector<char> &code);
    };

    Pipeline() = default;

    void destroy();

    VkDescriptorSet createDescriptorSet(
        const std::vector<DescriptorData> &descriptors
    );

    void bindDescriptorSet(VkDescriptorSet set);
    void bindDescriptorSets(
        const std::vector<VkDescriptorSet> &sets,
        u32 offset = 0
    );

    void bind();
    void push(
        VkShaderStageFlags stage,
        u32 offset,
        u32 size,
        const void *data
    );

private:
    friend class Builder;

    VulkanCtx *m_ctx;

    VkPipeline m_handle;
    VkPipelineLayout m_layout;
    VkDescriptorSetLayout m_descriptorLayout;
    VkDescriptorPool m_descriptorPool;

    f32 m_lineWidth;
};
    
} // namespace gfx