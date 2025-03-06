#include "button.hpp"
#include "gui.hpp"

namespace gui
{

Button::Button(
    GUI *gui,
    const std::string &text,
    const glm::vec2 &pos,
    const glm::vec2 &size,
    Anchor anchor
)
    : m_gui(gui), m_text(text)
{
    m_element = {
        .anchor = anchor,
        .position = pos,
        .size = size,
        .uv = NORMAL_UV,
        .texture = "gui",
    };
}

void Button::render(const glm::vec2 &point)
{
    bool wasHovered = m_hovered;
    m_hovered = countain(point);

    if (wasHovered != m_hovered) {
        m_element.uv = m_hovered ? HOVER_UV : NORMAL_UV;
    }

    if (!m_text.empty()) {
        m_gui->drawText(m_text, getAbsolutePos(), 24.0f, TextAlign::CENTER);
    }
}

bool Button::countain(const glm::vec2 &point) const
{
    glm::vec2 pos = getAbsolutePos();

    return 
        point.x >= pos.x &&
        point.x <= pos.x + m_element.size.x &&
        point.y >= pos.y &&
        point.y <= pos.y + m_element.size.y;
}

glm::vec2 Button::getAbsolutePos() const
{
    glm::vec2 pos = m_element.position;
    glm::vec2 size = m_element.size;

    VkExtent2D extent = m_gui->getExtent();

    switch (m_element.anchor)
    {

    case Anchor::TOP_LEFT:
        break;

    case Anchor::TOP_RIGHT:
        pos.x = extent.width - pos.x - size.x;
        break;

    case Anchor::BOTTOM_LEFT:
        pos.y = extent.height - pos.y - size.y;
        break;

    case Anchor::BOTTOM_RIGHT:
        pos.x = extent.width - pos.x - size.x;
        pos.y = extent.height - pos.y - size.y;
        break;

    case Anchor::CENTER:
        pos.x = extent.width / 2.0f - size.x / 2.0f;
        pos.y = extent.height / 2.0f - size.y / 2.0f;
        break;

    }

    return pos;
}

} // 