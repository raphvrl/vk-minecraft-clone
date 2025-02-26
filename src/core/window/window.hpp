#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>

#include <string>
#include <array>
#include <stdexcept>

#include "core/types.hpp"

namespace core
{

enum class Key {
    SPACE = GLFW_KEY_SPACE,
    APOSTROPHE = GLFW_KEY_APOSTROPHE,
    COMMA = GLFW_KEY_COMMA,
    MINUS = GLFW_KEY_MINUS,
    PERIOD = GLFW_KEY_PERIOD,
    SLASH = GLFW_KEY_SLASH,
    _0 = GLFW_KEY_0,
    _1 = GLFW_KEY_1,
    _2 = GLFW_KEY_2,
    _3 = GLFW_KEY_3,
    _4 = GLFW_KEY_4,
    _5 = GLFW_KEY_5,
    _6 = GLFW_KEY_6,
    _7 = GLFW_KEY_7,
    _8 = GLFW_KEY_8,
    _9 = GLFW_KEY_9,
    SEMICOLON = GLFW_KEY_SEMICOLON,
    EQUAL = GLFW_KEY_EQUAL,
    A = GLFW_KEY_A,
    B = GLFW_KEY_B,
    C = GLFW_KEY_C,
    D = GLFW_KEY_D,
    E = GLFW_KEY_E,
    F = GLFW_KEY_F,
    G = GLFW_KEY_G,
    H = GLFW_KEY_H,
    I = GLFW_KEY_I,
    J = GLFW_KEY_J,
    K = GLFW_KEY_K,
    L = GLFW_KEY_L,
    M = GLFW_KEY_M,
    N = GLFW_KEY_N,
    O = GLFW_KEY_O,
    P = GLFW_KEY_P,
    Q = GLFW_KEY_Q,
    R = GLFW_KEY_R,
    S = GLFW_KEY_S,
    T = GLFW_KEY_T,
    U = GLFW_KEY_U,
    V = GLFW_KEY_V,
    W = GLFW_KEY_W,
    X = GLFW_KEY_X,
    Y = GLFW_KEY_Y,
    Z = GLFW_KEY_Z,
    ESCAPE = GLFW_KEY_ESCAPE,
    ENTER = GLFW_KEY_ENTER,
    TAB = GLFW_KEY_TAB,
    BACKSPACE = GLFW_KEY_BACKSPACE,
    DELETE = GLFW_KEY_DELETE,
    RIGHT = GLFW_KEY_RIGHT,
    LEFT = GLFW_KEY_LEFT,
    DOWN = GLFW_KEY_DOWN,
    UP = GLFW_KEY_UP,
    LSHIFT = GLFW_KEY_LEFT_SHIFT,
    RSHIFT = GLFW_KEY_RIGHT_SHIFT,
    LCONTROL = GLFW_KEY_LEFT_CONTROL,
    RCONTROL = GLFW_KEY_RIGHT_CONTROL,
    LALT = GLFW_KEY_LEFT_ALT,
    RALT = GLFW_KEY_RIGHT_ALT,
    F1 = GLFW_KEY_F1,
    F2 = GLFW_KEY_F2,
    F3 = GLFW_KEY_F3,
    F4 = GLFW_KEY_F4,
    F5 = GLFW_KEY_F5,
    F6 = GLFW_KEY_F6,
    F7 = GLFW_KEY_F7,
    F8 = GLFW_KEY_F8,
    F9 = GLFW_KEY_F9,
    F10 = GLFW_KEY_F10,
    F11 = GLFW_KEY_F11,
    F12 = GLFW_KEY_F12,
    F13 = GLFW_KEY_F13,
    F14 = GLFW_KEY_F14,
    F15 = GLFW_KEY_F15,
    F16 = GLFW_KEY_F16,
    F17 = GLFW_KEY_F17,
    F18 = GLFW_KEY_F18,
    F19 = GLFW_KEY_F19,
    F20 = GLFW_KEY_F20,
    F21 = GLFW_KEY_F21,
    F22 = GLFW_KEY_F22,
    F23 = GLFW_KEY_F23,
    F24 = GLFW_KEY_F24,
    F25 = GLFW_KEY_F25
};

enum class MouseButton {
    LEFT = GLFW_MOUSE_BUTTON_LEFT,
    RIGHT = GLFW_MOUSE_BUTTON_RIGHT,
    MIDDLE = GLFW_MOUSE_BUTTON_MIDDLE
};

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

    void setCursorVisible(bool visible);

    bool isOpen() const { return !glfwWindowShouldClose(m_handle); }

    bool isKeyPressed(Key k) const { return m_keys[static_cast<int>(k)]; }
    bool isMouseButtonPressed(MouseButton b) const { 
        return m_mouseButtons[static_cast<int>(b)]; 
    }

    GLFWwindow *get() const { return m_handle; }

    u32 getWidth() const { return m_width; }
    u32 getHeight() const { return m_height; }
    f32 getAspect() const { return static_cast<f32>(m_width) / m_height; }

    f32 getDeltaTime() const { return m_deltaTime; }

    glm::vec2 getMousePos() const { return m_mousePos; }
    glm::vec2 getMouseRel() const { return m_mouseRel; }

private:
    GLFWwindow *m_handle;
    u32 m_width;
    u32 m_height;
    std::string m_title;

    f32 m_lastFrame;
    f32 m_deltaTime;

    std::array<bool, GLFW_KEY_LAST + 1> m_keys;
    std::array<bool, GLFW_MOUSE_BUTTON_LAST + 1> m_mouseButtons;
    
    glm::vec2 m_mousePos;
    glm::vec2 m_mouseRel;
    bool m_firstMouse = true;

    bool m_resized = false;

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
};

} // namespace core