#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>

#include <string>
#include <array>
#include <stdexcept>
#include <functional>

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

    void close() { glfwSetWindowShouldClose(m_handle, GLFW_TRUE); }

    void setCursorMode(int mode) {
        glfwSetInputMode(m_handle, GLFW_CURSOR, mode);
    }

    void setFullscreen(bool fullscreen);
    void toogleFullscreen() {
        setFullscreen(!glfwGetWindowMonitor(m_handle));
    }

    bool isOpen() const { return !glfwWindowShouldClose(m_handle); }

    bool isKeyPressed(int k) const { return m_keys[k]; }
    bool isKeyJustPressed(int k) const {
        return m_keys[static_cast<int>(k)] && !m_keysPrev[static_cast<int>(k)];
    }

    bool isMouseButtonPressed(int b) const { return m_mouseButtons[b]; }

public:

    GLFWwindow *get() const { return m_handle; }

    u32 getWidth() const { return m_width; }
    u32 getHeight() const { return m_height; }
    f32 getAspect() const { return static_cast<f32>(m_width) / m_height; }

    f32 getCurrentTime() const { return glfwGetTime(); }
    f32 getDeltaTime() const { return m_deltaTime; }

    glm::vec2 getMousePos() const { return m_mousePos; }
    glm::vec2 getMouseRel() const { return m_mouseRel; }

    bool isResized() const { return m_resized; }
    bool isMinimized() const { return m_minimized; }

private:
    GLFWwindow *m_handle;
    u32 m_width;
    u32 m_height;
    int m_x;
    int m_y;
    bool m_fullscreen = false;
    std::string m_title;

    f32 m_lastFrame;
    f32 m_deltaTime;

    std::array<bool, GLFW_KEY_LAST + 1> m_keys;
    std::array<bool, GLFW_KEY_LAST + 1> m_keysPrev;
    std::array<bool, GLFW_MOUSE_BUTTON_LAST + 1> m_mouseButtons;
    
    glm::vec2 m_mousePos;
    glm::vec2 m_mouseRel;
    bool m_firstMouse = true;

    bool m_resized = false;

    bool m_minimized = false;

    static void keyCallback(
        GLFWwindow *window,
        int key,
        int scancode,
        int action,
        int mods
    );

    static void mouseButtonCallback(
        GLFWwindow *window,
        int button,
        int action,
        int mods
    );

    static void mousePosCallback(
        GLFWwindow *window,
        double x,
        double y
    );

    static void resizeCallback(
        GLFWwindow *window,
        int width,
        int height
    );

private:
    GLFWmonitor *getCurrentMonitor();

};

} // namespace core