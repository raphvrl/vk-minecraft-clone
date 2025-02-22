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

    m_keys.fill(false);
    m_mouseButtons.fill(false);

    glfwSetWindowUserPointer(m_handle, this);

    glfwSetKeyCallback(m_handle, keyCallback);
    glfwSetMouseButtonCallback(m_handle, mouseButtonCallback);
    glfwSetCursorPosCallback(m_handle, mousePosCallback);
}

void Window::destroy()
{
    glfwDestroyWindow(m_handle);
    glfwTerminate();
}

void Window::update()
{
    m_mouseRel = glm::vec2(0.0f);

    glfwPollEvents();

    f32 currentFrame = static_cast<f32>(glfwGetTime());
    m_deltaTime = currentFrame - m_lastFrame;
    m_lastFrame = currentFrame;
}

void Window::setCursorVisible(bool visible)
{
    glfwSetInputMode(
        m_handle,
        GLFW_CURSOR,
        visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED
    );
}

void Window::keyCallback(
    GLFWwindow *window,
    int key,
    int scancode,
    int action,
    int mods
)
{
    UNUSED(scancode);
    UNUSED(mods);

    Window *win = static_cast<Window *>(glfwGetWindowUserPointer(window));

    if (action == GLFW_PRESS) {
        win->m_keys[key] = true;
    } else if (action == GLFW_RELEASE) {
        win->m_keys[key] = false;
    }
}

void Window::mouseButtonCallback(
    GLFWwindow *window,
    int button,
    int action,
    int mods
)
{
    UNUSED(mods);

    Window *win = static_cast<Window *>(glfwGetWindowUserPointer(window));

    if (action == GLFW_PRESS) {
        win->m_mouseButtons[button] = true;
    } else if (action == GLFW_RELEASE) {
        win->m_mouseButtons[button] = false;
    }
}

void Window::mousePosCallback(
    GLFWwindow *window,
    double x,
    double y
)
{
    Window *win = static_cast<Window *>(glfwGetWindowUserPointer(window));

    if (win->m_firstMouse) {
        win->m_mousePos = glm::vec2(x, y);
        win->m_firstMouse = false;
    }

    win->m_mouseRel = glm::vec2(
        x - win->m_mousePos.x,
        win->m_mousePos.y - y
    );

    win->m_mousePos = glm::vec2(x, y);
}

} // namespace core