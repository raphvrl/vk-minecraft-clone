#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>
#include <filesystem>
#include <fstream>

#include "device.hpp"
#include "utils/utils.hpp"

namespace fs = std::filesystem;

constexpr const char* SHADER_DIR_STR = "assets/shaders/";

namespace gfx
{

struct VertexInput
{
    VkVertexInputBindingDescription *binding = {};
    VkVertexInputAttributeDescription *attribute = {};
    u32 attributeCount = 0;
};

class Pipeline
{

public:
    class Builder
    {

    public:
        Builder(Device &device);
        ~Builder() = default;

        Builder &setShader(const fs::path &filename, VkShaderStageFlagBits stage);
        Builder &setVertexInput(const VertexInput &vertexInput);
        Builder &setColorFormat(VkFormat format);
        Builder &setPushConstant(u32 size);
        Builder &setDepthTest(bool enable);
        Builder &setDepthWrite(bool enable);
        Builder &setAlphaTest(bool enable);
        Builder &setCullMode(VkCullModeFlags mode);
        Builder &setCull(bool enable);
        Builder &setBlending(bool enable);
        Builder &setTopology(VkPrimitiveTopology topology);
        Builder &setLineWidth(f32 width);

        Pipeline build();

    private:
        Device &m_device;

        std::vector<VkPipelineShaderStageCreateInfo> m_shaderStages;

        VertexInput m_vertexInput = {};
        bool m_vertexInputSet = false;

        VkFormat m_colorFormat = VK_FORMAT_B8G8R8A8_UNORM;

        VkPushConstantRange m_pushConstantRanges;
        bool m_pushConstantSet = false;

        bool m_depthTest = false;
        bool m_depthWrite = false;

        bool m_alphaTest = false;

        bool m_cull = false;
        VkCullModeFlags m_cullMode = VK_CULL_MODE_BACK_BIT;

        bool m_blending = false;

        VkPrimitiveTopology m_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        f32 m_lineWidth = 1.0f;

        std::vector<char> readFile(const fs::path &filename);
        VkShaderModule createShaderModule(const std::vector<char> &code);

    };

    Pipeline() = default;
    void destroy();

    void bind(VkCommandBuffer cmd);

    template<typename T>
    void push(VkCommandBuffer cmd, T &data);

private:
    friend class Builder;

    Device *m_device;
    VkPipeline m_pipeline;
    VkPipelineLayout m_pipelineLayout;
    VkDescriptorSet m_descriptorSet;

    f32 m_lineWidth = 1.0f;

};

template<typename T>
void Pipeline::push(VkCommandBuffer cmd, T &data)
{
    vkCmdPushConstants(
        cmd,
        m_pipelineLayout,
        VK_SHADER_STAGE_ALL_GRAPHICS,
        0,
        sizeof(T),
        reinterpret_cast<void *>(&data)
    );
}

} // namespace gfx