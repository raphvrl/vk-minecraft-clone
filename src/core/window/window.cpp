#include "window.hpp"

namespace core
{

void Window::init(u32 width, u32 height, const std::string &title)
{
    m_width = width;
    m_height = height;
    m_title = title;

    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_handle = glfwCreateWindow(
        static_cast<int>(m_width),
        static_cast<int>(m_height),
        m_title.c_str(),
        nullptr,
        nullptr
    );

    if (!m_handle) {
        glfwTerminate();
        throw std::runtime_error("Failed to create window");
    }
}

void Window::destroy()
{
    glfwDestroyWindow(m_handle);
    glfwTerminate();
}

void Window::update()
{
    glfwPollEvents();
}

} // namespace core