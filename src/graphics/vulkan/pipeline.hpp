#pragma once

#include <vector>
#include <string>
#include <string_view>
#include <fstream>
#include <stdexcept>

#include "vulkan_ctx.hpp"

namespace gfx
{

constexpr std::string_view SHADER_PATH = "bin/shaders/";

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

        Pipeline build();

    private:
        VulkanCtx& m_ctx;

        std::map<int, std::string> m_shaderPaths;
        std::vector<VkPushConstantRange> m_pushConstants;

        std::vector<char> readFile(const std::string &path);
        VkShaderModule createShaderModule(const std::vector<char> &code);
    };

    Pipeline() = default;

    void destroy();

    void bind();
    void push(ShaderType type, u32 offset, u32 size, const void *data);

private:
    friend class Builder;

    VulkanCtx *m_ctx;

    VkPipeline m_handle;
    VkPipelineLayout m_layout;
};
    
} // namespace gfx