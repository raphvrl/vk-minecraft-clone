#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <array>
#include <stdexcept>

#include "core/types.hpp"

namespace core
{

class Window
{

public:
    Window() = default;
    virtual ~Window() = default;

    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;

    void init(u32 width, u32 height, const std::string &title);
    void destroy();

    void update();

    bool isOpen() const { return !glfwWindowShouldClose(m_handle); }

    GLFWwindow *get() const { return m_handle; }

    u32 getWidth() const { return m_width; }
    u32 getHeight() const { return m_height; }

private:
    GLFWwindow *m_handle;
    u32 m_width;
    u32 m_height;
    std::string m_title;

    
};

} // namespace core