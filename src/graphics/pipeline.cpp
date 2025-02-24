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

Pipeline::Builder &Pipeline::Builder::addPushConstant(
    ShaderType type,
    u32 offset,
    u32 size
)
{
    VkPushConstantRange pushConstantRange = {};
    pushConstantRange.stageFlags = type == ShaderType::VERTEX
        ? VK_SHADER_STAGE_VERTEX_BIT
        : VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = offset;
    pushConstantRange.size = size;

    m_pushConstants.push_back(pushConstantRange);
    return *this;
}

Pipeline::Builder &Pipeline::Builder::setVertexInput(
    const VkVertexInputBindingDescription *bindingDescription,
    const VkVertexInputAttributeDescription *attributeDescriptions,
    u32 attributeCount
)
{
    m_hasVertexInput = true;

    m_bindingDescription = *bindingDescription;

    m_attributeDescriptions.clear();
    m_attributeDescriptions.resize(attributeCount);

    for (u32 i = 0; i < attributeCount; i++) {
        m_attributeDescriptions[i] = attributeDescriptions[i];
    }

    return *this;
}

Pipeline::Builder &Pipeline::Builder::addDescriptorBinding(
    const DescriptorLayout &layout
)
{
    m_descriptorLayouts.push_back(layout);
    return *this;
}

Pipeline::Builder &Pipeline::Builder::setDepthTest(bool enable)
{
    m_depthTest = enable;
    return *this;
}

Pipeline::Builder &Pipeline::Builder::setDepthWrite(bool enable)
{
    m_depthWrite = enable;
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

    if (m_hasVertexInput) {
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &m_bindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<u32>(m_attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = m_attributeDescriptions.data();
    } else {
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.pVertexBindingDescriptions = nullptr;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
        vertexInputInfo.pVertexAttributeDescriptions = nullptr;
    }
    

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
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
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

    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = m_depthTest ? VK_TRUE : VK_FALSE;
    depthStencil.depthWriteEnable = m_depthWrite ? VK_TRUE : VK_FALSE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.back = {};

    std::vector<VkDescriptorSetLayoutBinding> bindings;
    bindings.reserve(m_descriptorLayouts.size());

    for (const auto &layout : m_descriptorLayouts) {
        VkDescriptorSetLayoutBinding binding = {};
        binding.binding = layout.binding;
        binding.descriptorType = layout.type;
        binding.descriptorCount = layout.count;
        binding.stageFlags = layout.stage;
        binding.pImmutableSamplers = nullptr;

        bindings.push_back(binding);
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<u32>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    VkDescriptorSetLayout descriptorLayout;
    VkResult res = vkCreateDescriptorSetLayout(
        m_ctx.getDevice(),
        &layoutInfo,
        nullptr,
        &descriptorLayout
    );

    VulkanCtx::check(res, "Failed to create descriptor set layout!");

    std::vector<VkDescriptorPoolSize> poolSizes;
    poolSizes.reserve(m_descriptorLayouts.size());

    for (const auto &layout : m_descriptorLayouts) {
        VkDescriptorPoolSize poolSize = {};
        poolSize.type = layout.type;
        poolSize.descriptorCount = layout.count;

        poolSizes.push_back(poolSize);
    }

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<u32>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 1;

    VkDescriptorPool descriptorPool;
    res = vkCreateDescriptorPool(
        m_ctx.getDevice(),
        &poolInfo,
        nullptr,
        &descriptorPool
    );

    VulkanCtx::check(res, "Failed to create descriptor pool!");

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pushConstantRangeCount =  static_cast<u32>(
        m_pushConstants.size()
    );
    pipelineLayoutInfo.pPushConstantRanges = m_pushConstants.data();
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorLayout;

    VkPipelineLayout pipelineLayout;
    res = vkCreatePipelineLayout(
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
    pipelineInfo.pDepthStencilState = &depthStencil;
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
    PipelineObj.m_descriptor = descriptorLayout;
    PipelineObj.m_descriptorPool = descriptorPool;

    return PipelineObj;
}

std::vector<char> Pipeline::Builder::readFile(const std::string &path)
{
    std::string fullPath = std::string(SHADER_PATH) + path;

    std::ifstream file(fullPath, std::ios::ate | std::ios::binary);

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

    vkDestroyDescriptorPool(m_ctx->getDevice(), m_descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(m_ctx->getDevice(), m_descriptor, nullptr);
    vkDestroyPipeline(m_ctx->getDevice(), m_handle, nullptr);
    vkDestroyPipelineLayout(m_ctx->getDevice(), m_layout, nullptr);
}

VkDescriptorSet Pipeline::createDescriptorSet(Texture &texture)
{
    VkDescriptorSetLayout layouts[] = {m_descriptor};

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = layouts;

    VkDescriptorSet descriptorSet;
    VkResult res = vkAllocateDescriptorSets(
        m_ctx->getDevice(),
        &allocInfo,
        &descriptorSet
    );

    VulkanCtx::check(res, "Failed to allocate descriptor set!");

    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = texture.getImageView();
    imageInfo.sampler = texture.getSampler();

    VkWriteDescriptorSet descriptorWrite = {};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(
        m_ctx->getDevice(),
        1,
        &descriptorWrite,
        0,
        nullptr
    );

    return descriptorSet;
}

void Pipeline::bindDescriptorSet(VkDescriptorSet set)
{
    vkCmdBindDescriptorSets(
        m_ctx->getCommandBuffer(),
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_layout,
        0,
        1,
        &set,
        0,
        nullptr
    );
}

void Pipeline::bind()
{
    vkCmdBindPipeline(
        m_ctx->getCommandBuffer(),
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_handle
    );
}

void Pipeline::push(ShaderType type, u32 offset, u32 size, const void *data)
{
    vkCmdPushConstants(
        m_ctx->getCommandBuffer(),
        m_layout,
        type == ShaderType::VERTEX
            ? VK_SHADER_STAGE_VERTEX_BIT
            : VK_SHADER_STAGE_FRAGMENT_BIT,
        offset,
        size,
        data
    );
}

} // namespace gfx