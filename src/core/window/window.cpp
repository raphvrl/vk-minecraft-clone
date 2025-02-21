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

    m_window = glfwCreateWindow(
        static_cast<int>(m_width),
        static_cast<int>(m_height),
        m_title.c_str(),
        nullptr,
        nullptr
    );

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    if (!m_window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create window");
    }
}

void Window::destroy()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void Window::update()
{
    glfwPollEvents();
}

} // namespace core