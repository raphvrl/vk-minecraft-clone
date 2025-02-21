#pragma once

#include <vulkan/vulkan.h>

#include <string>

#include "core/window/window.hpp"

namespace gfx
{
    
class VulkanCtx
{

public:
    VulkanCtx() = delete;
    virtual ~VulkanCtx();

private:
    core::Window *m_window;

    VkInstance m_instance;
    VkDebugUtilsMessengerEXT m_debugMessenger;

};

} // namespace gfx