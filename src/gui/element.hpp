#pragma once

#include <glm/ext.hpp>

namespace gui
{

enum class Anchor
{
    TOP_LEFT,
    TOP_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_RIGHT,
    CENTER,
};

struct Element
{
    Anchor anchor;
    glm::vec2 position;
    glm::vec2 size;
    glm::vec4 uv;
    std::string texture;
};

} // namespace gui