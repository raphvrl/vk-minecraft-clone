#include "button.hpp"
#include "gui.hpp"

namespace gui
{

Button::Button(
    GUI *gui,
    const std::string &text,
    Element element,
    CallBackFn callback
)
    : m_gui(gui), m_text(text)
{
    m_element = element;
    m_element.uv = NORMAL_UV;

    m_callback = callback;
}

void Button::update(const glm::vec2 &point)
{
    m_hovered = countain(point);

    if (m_pressed) {
        m_pressed = false;
    }
}

void Button::handleMouseClick()
{
    if (m_hovered && !m_pressed) {
        m_callback();
        m_pressed = true;
    }
}

void Button::render()
{
    if (m_hovered) {
        m_element.uv = HOVER_UV;
    } else {
        m_element.uv = NORMAL_UV;
    }

    m_gui->draw(m_element);

    glm::vec2 textPos = getAbsolutePos();

    textPos.x += m_element.size.x / 2.0f;
    textPos.y += m_element.size.y / 2.0f - 12.0f;

    m_gui->drawText(m_text, textPos, 24.0f, TextAlign::CENTER);
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
    glm::vec2 pos = m_element.pos;
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
        pos.x = extent.width / 2.0f - size.x / 2.0f + m_element.pos.x;
        pos.y = extent.height / 2.0f - size.y / 2.0f + m_element.pos.y;
        break;
    }

    return pos;
}

} // 