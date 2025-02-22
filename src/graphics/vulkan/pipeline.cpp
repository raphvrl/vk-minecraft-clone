#include "pipeline.hpp"

namespace gfx
{

Pipeline::Builder::Builder(VulkanCtx &ctx)
    : m_ctx(ctx)
{
}

Pipeline::Builder &Pipeline::Builder::setShader(
    ShaderType type,
    const std::string &path
)
{
    m_shaderPaths[static_cast<int>(type)] = path;
    return *this;
}

Pipeline Pipeline::Builder::build()
{
    auto vertexCode = readFile(
        m_shaderPaths[static_cast<int>(ShaderType::VERTEX)]
    );

    VkShaderModule vertexModule = createShaderModule(vertexCode);
    
    auto fragmentCode = readFile(
        m_shaderPaths[static_cast<int>(ShaderType::FRAGMENT)]
    );

    VkShaderModule fragmentModule = createShaderModule(fragmentCode);

    VkPipelineShaderStageCreateInfo shaderStage[2] = {};

    shaderStage[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStage[0].module = vertexModule;
    shaderStage[0].pName = "main";

    shaderStage[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStage[1].module = fragmentModule;
    shaderStage[1].pName = "main";

    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_ctx.getSwapChainExtent().width);
    viewport.height = static_cast<float>(m_ctx.getSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = m_ctx.getSwapChainExtent();

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = 
        VK_COLOR_COMPONENT_R_BIT | 
        VK_COLOR_COMPONENT_G_BIT | 
        VK_COLOR_COMPONENT_B_BIT | 
        VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    VkPipelineLayout pipelineLayout;
    VkResult res = vkCreatePipelineLayout(
        m_ctx.getDevice(),
        &pipelineLayoutInfo,
        nullptr,
        &pipelineLayout
    );

    VulkanCtx::check(res, "Failed to create pipeline layout!");

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStage;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = m_ctx.getRenderPass();
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.subpass = 0;


    VkPipeline pipeline;
    res = vkCreateGraphicsPipelines(
        m_ctx.getDevice(),
        VK_NULL_HANDLE,
        1,
        &pipelineInfo,
        nullptr,
        &pipeline
    );

    VulkanCtx::check(res, "Failed to create graphics pipeline!");

    vkDestroyShaderModule(m_ctx.getDevice(), vertexModule, nullptr);
    vkDestroyShaderModule(m_ctx.getDevice(), fragmentModule, nullptr);

    Pipeline PipelineObj;
    PipelineObj.m_ctx = &m_ctx;
    PipelineObj.m_handle = pipeline;
    PipelineObj.m_layout = pipelineLayout;

    return PipelineObj;
}

std::vector<char> Pipeline::Builder::readFile(const std::string &path)
{
    std::ifstream file(path, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + path);
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);
    file.seekg(0);

    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

VkShaderModule Pipeline::Builder::createShaderModule(const std::vector<char> &code)
{
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule;
    VkResult res = vkCreateShaderModule(
        m_ctx.getDevice(),
        &createInfo,
        nullptr,
        &shaderModule
    );

    VulkanCtx::check(res, "Failed to create shader module!");

    return shaderModule;
}

void Pipeline::destroy()
{
    vkDeviceWaitIdle(m_ctx->getDevice());

    vkDestroyPipeline(m_ctx->getDevice(), m_handle, nullptr);
    vkDestroyPipelineLayout(m_ctx->getDevice(), m_layout, nullptr);
}

void Pipeline::bind()
{
    vkCmdBindPipeline(
        m_ctx->getCommandBuffer(),
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_handle
    );
}

} // namespace gfx