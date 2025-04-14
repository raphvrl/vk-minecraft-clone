#include "imgui_manager.hpp"

namespace adm
{

void ImGuiManager::init(gfx::Device &device, core::Window &window)
{
    m_device = &device;

    m_descriptorPool = createDescriptorPool();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(window.get(), true);

    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.Instance = device.getInstance();
    initInfo.PhysicalDevice = device.getPhysicalDevice();
    initInfo.Device = device.getDevice();
    initInfo.QueueFamily = device.getGraphicsQueueFamilyIndex();
    initInfo.Queue = device.getGraphicsQueue();
    initInfo.PipelineCache = VK_NULL_HANDLE;
    initInfo.DescriptorPool = m_descriptorPool;
    initInfo.MinImageCount = 2;
    initInfo.ImageCount = device.getSwapchain().getImages().size();
    initInfo.Allocator = nullptr;
    initInfo.UseDynamicRendering = true;
    initInfo.RenderPass = VK_NULL_HANDLE;
    initInfo.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    initInfo.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
    VkFormat colorFormat = device.getSwapchain().getFormat();
    initInfo.PipelineRenderingCreateInfo.pColorAttachmentFormats = &colorFormat;
    VkFormat depthFormat = device.getDepthFormat();
    initInfo.PipelineRenderingCreateInfo.depthAttachmentFormat = depthFormat;
    initInfo.PipelineRenderingCreateInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

    if (!ImGui_ImplVulkan_Init(&initInfo)) {
        throw std::runtime_error("Failed to initialize ImGui Vulkan backend");
    } 

    ImGui_ImplVulkan_CreateFontsTexture();
}

void ImGuiManager::destroy()
{
    ImGui_ImplVulkan_DestroyFontsTexture();
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    vkDestroyDescriptorPool(m_device->getDevice(), m_descriptorPool, nullptr);
}

void ImGuiManager::beginFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiManager::endFrame(VkCommandBuffer cmd)
{
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
}

VkDescriptorPool ImGuiManager::createDescriptorPool()
{
    VkDescriptorPoolSize poolSizes[] = {
        {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}
    };

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1000 * IM_ARRAYSIZE(poolSizes);
    poolInfo.poolSizeCount = static_cast<u32>(std::size(poolSizes));
    poolInfo.pPoolSizes = poolSizes;

    VkDescriptorPool descriptorPool;
    vkCreateDescriptorPool(
        m_device->getDevice(),
        &poolInfo,
        nullptr,
        &descriptorPool
    );

    return descriptorPool;
}

} // namespace adm