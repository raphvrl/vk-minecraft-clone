#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>

#include "vulkan_ctx.hpp"

namespace gfx
{

enum class ShaderType
{
    VERTEX,
    FRAGMENT,
    GEOMETRY
};

class Pipeline
{

public:
    class Builder
    {

    public:
        Builder(VulkanCtx &ctx);

        Builder &setShader(ShaderType type, const std::string &path);

        Pipeline build();

    private:
        VulkanCtx& m_ctx;

        std::map<int, std::string> m_shaderPaths;

        std::vector<char> readFile(const std::string &path);
        VkShaderModule createShaderModule(const std::vector<char> &code);
    };

    Pipeline() = default;

    void destroy();

    void bind();

private:
    friend class Builder;

    VulkanCtx *m_ctx;

    VkPipeline m_handle;
    VkPipelineLayout m_layout;
};
    
} // namespace gfx