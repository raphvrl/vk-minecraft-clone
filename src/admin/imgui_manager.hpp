#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "graphics/device.hpp"

namespace adm
{

class ImGuiManager
{

public:
    ImGuiManager() = default;
    ~ImGuiManager() = default;

    void init(gfx::Device &device, core::Window &window);
    void destroy();

    void beginFrame();
    void endFrame(VkCommandBuffer cmd);

private:
    gfx::Device *m_device = nullptr;

    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;

    VkDescriptorPool createDescriptorPool();

};

} // namespace adm
