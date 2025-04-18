#pragma once

#include <nlohmann/json.hpp>

#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <functional>

#include "game/game_state.hpp"

using json = nlohmann::json;

namespace gui
{

enum class ElementType
{
    PANEL,
    BUTTON,
    LABEL,
    IMAGE,
    NAVBAR
};

enum class ElementAnchor
{
    TOP_LEFT,
    TOP_CENTER,
    TOP_RIGHT,
    CENTER_LEFT,
    CENTER,
    CENTER_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_CENTER,
    BOTTOM_RIGHT
};

struct GUIElement
{
    std::string id;
    ElementType type;
    ElementAnchor anchor;
    glm::vec2 position;
    glm::vec2 size;
    game::GameState state;
    bool visible;
    std::string texture;
    glm::vec4 uv;
    std::string text;
    bool invertEffect;

    std::function<void()> onClick = nullptr;

    static std::shared_ptr<GUIElement> fromJson(const json &json);
    json toJson() const;
};

std::string elementTypeToString(ElementType type);
ElementType stringToElementType(const std::string &str);

std::string elementAnchorToString(ElementAnchor anchor);
ElementAnchor stringToElementAnchor(const std::string &str);

std::string elementStateToString(game::GameState state);
game::GameState stringToElementState(const std::string &str);

} // namespace gui