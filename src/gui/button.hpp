#pragma once

#include <glm/glm.hpp>

#include <string>

#include "element.hpp"

namespace gui
{

class GUI;

class Button
{

public:
    Button(
        GUI *gui,
        const std::string &text,
        const glm::vec2 &pos,
        const glm::vec2 &size,
        Anchor anchor = Anchor::CENTER
    );

    void render(const glm::vec2 &point);

    bool countain(const glm::vec2 &point) const;

private:
    GUI *m_gui;
    Element m_element;
    std::string m_text;

    bool m_hovered = false;
    bool m_pressed = false;

    static constexpr glm::vec4 NORMAL_UV = {0.0f, 67.0f, 200.0f, 20.0f};
    static constexpr glm::vec4 HOVER_UV = {0.0f, 87.0f, 200.0f, 20.0f};

    glm::vec2 getAbsolutePos() const;
};

} // namespace gui