#pragma once

#include <glm/glm.hpp>

#include <string>
#include <functional>

#include "element.hpp"

namespace gui
{

class GUI;

class Button
{

public:
    using CallBackFn = std::function<void()>;

    Button(
        GUI *gui,
        const std::string &text,
        Element element
    );        

    void render(const glm::vec2 &point);

    bool countain(const glm::vec2 &point) const;

private:
    GUI *m_gui;
    Element m_element;
    std::string m_text;

    bool m_hovered = false;
    bool m_pressed = false;

    CallBackFn m_callback = nullptr;

    static constexpr glm::vec4 NORMAL_UV = {0.0f, 66.0f, 200.0f, 20.0f};
    static constexpr glm::vec4 HOVER_UV = {0.0f, 86.0f, 200.0f, 20.0f};

    glm::vec2 getAbsolutePos() const;
};

} // namespace gui