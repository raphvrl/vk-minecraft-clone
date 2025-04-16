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

    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    if (mode) {
        int xpos = (mode->width - static_cast<int>(m_width)) / 2;
        int ypos = (mode->height - static_cast<int>(m_height)) / 2;
 
        glfwSetWindowPos(m_handle, xpos, ypos);
    }

    m_keys.fill(false);
    m_keysPrev.fill(false);
    m_mouseButtons.fill(false);

    glfwSetWindowUserPointer(m_handle, this);

    glfwSetKeyCallback(m_handle, keyCallback);
    glfwSetMouseButtonCallback(m_handle, mouseButtonCallback);
    glfwSetCursorPosCallback(m_handle, mousePosCallback);
    glfwSetFramebufferSizeCallback(m_handle, resizeCallback);
}

void Window::destroy()
{
    glfwDestroyWindow(m_handle);
    glfwTerminate();
}

void Window::update()
{
    m_mouseRel = glm::vec2(0.0f);

    m_keysPrev = m_keys;
    
    f32 currentFrame = static_cast<f32>(glfwGetTime());
    m_deltaTime = currentFrame - m_lastFrame;
    m_lastFrame = currentFrame;

    glfwPollEvents();

    int width, height;
    glfwGetFramebufferSize(m_handle, &width, &height);
    m_minimized = (width == 0 || height == 0);
}

void Window::setFullscreen(bool fullscreen)
{
    if (fullscreen == m_fullscreen) {
        return;
    }

    m_fullscreen = fullscreen;

    if (fullscreen) {
        glfwGetWindowPos(m_handle, &m_x, &m_y);
        m_width = static_cast<u32>(m_width);
        m_height = static_cast<u32>(m_height);

        GLFWmonitor* currentMonitor = getCurrentMonitor();

        const GLFWvidmode* mode = glfwGetVideoMode(currentMonitor);

        glfwSetWindowMonitor(
            m_handle,
            currentMonitor,
            0,
            0,
            mode->width,
            mode->height,
            mode->refreshRate
        );
    }
    else {
        glfwSetWindowMonitor(
            m_handle,
            nullptr,
            m_x,
            m_y,
            static_cast<int>(m_width),
            static_cast<int>(m_height),
            0
        );
    }
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

void Window::resizeCallback(
    GLFWwindow *window,
    int width,
    int height
)
{
    Window *win = static_cast<Window *>(glfwGetWindowUserPointer(window));

    win->m_width = static_cast<u32>(width);
    win->m_height = static_cast<u32>(height);

    win->m_resized = true;
}

GLFWmonitor *Window::getCurrentMonitor()
{
    int x, y, width, height;
    glfwGetWindowPos(m_handle, &x, &y);
    glfwGetWindowSize(m_handle, &width, &height);

    int monitorCount;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

    int centerX = x + width / 2;
    int centerY = y + height / 2;

    for (int i = 0; i < monitorCount; i++) {
        int mx, my, mwidth, mheight;
        glfwGetMonitorWorkarea(monitors[i], &mx, &my, &mwidth, &mheight);

        if (
            centerX >= mx &&
            centerX <= mx + mwidth &&
            centerY >= my &&
            centerY <= my + mheight
        ) {
            return monitors[i];
        }
    }

    return glfwGetPrimaryMonitor();
}

} // namespace core