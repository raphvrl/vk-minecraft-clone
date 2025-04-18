#include "gui_element.hpp"

namespace gui
{

std::string elementTypeToString(ElementType type)
{
    switch (type) {
        case ElementType::PANEL: return "PANEL";
        case ElementType::BUTTON: return "BUTTON";
        case ElementType::LABEL: return "LABEL";
        case ElementType::IMAGE: return "IMAGE";
        case ElementType::NAVBAR: return "NAVBAR";
        default: return "UNKNOWN";
    }
}

ElementType stringToElementType(const std::string &str)
{
    if (str == "PANEL") return ElementType::PANEL;
    if (str == "BUTTON") return ElementType::BUTTON;
    if (str == "LABEL") return ElementType::LABEL;
    if (str == "IMAGE") return ElementType::IMAGE;
    if (str == "NAVBAR") return ElementType::NAVBAR;
    return ElementType::PANEL;
}

std::string elementAnchorToString(ElementAnchor anchor)
{
    switch (anchor) {
        case ElementAnchor::TOP_LEFT: return "TOP_LEFT";
        case ElementAnchor::TOP_CENTER: return "TOP_CENTER";
        case ElementAnchor::TOP_RIGHT: return "TOP_RIGHT";
        case ElementAnchor::CENTER_LEFT: return "CENTER_LEFT";
        case ElementAnchor::CENTER: return "CENTER";
        case ElementAnchor::CENTER_RIGHT: return "CENTER_RIGHT";
        case ElementAnchor::BOTTOM_LEFT: return "BOTTOM_LEFT";
        case ElementAnchor::BOTTOM_CENTER: return "BOTTOM_CENTER";
        case ElementAnchor::BOTTOM_RIGHT: return "BOTTOM_RIGHT";
        default: return "UNKNOWN";
    }
}

ElementAnchor stringToElementAnchor(const std::string &str)
{
    if (str == "TOP_LEFT") return ElementAnchor::TOP_LEFT;
    if (str == "TOP_CENTER") return ElementAnchor::TOP_CENTER;
    if (str == "TOP_RIGHT") return ElementAnchor::TOP_RIGHT;
    if (str == "CENTER_LEFT") return ElementAnchor::CENTER_LEFT;
    if (str == "CENTER") return ElementAnchor::CENTER;
    if (str == "CENTER_RIGHT") return ElementAnchor::CENTER_RIGHT;
    if (str == "BOTTOM_LEFT") return ElementAnchor::BOTTOM_LEFT;
    if (str == "BOTTOM_CENTER") return ElementAnchor::BOTTOM_CENTER;
    if (str == "BOTTOM_RIGHT") return ElementAnchor::BOTTOM_RIGHT;
    return ElementAnchor::TOP_LEFT;
}

std::string elementStateToString(game::GameState state)
{
    switch (state) {
        case game::GameState::RUNNING: return "RUNNING";
        case game::GameState::PAUSED: return "PAUSED";
        default: return "UNKNOWN";
    }
}

game::GameState stringToElementState(const std::string &str)
{
    if (str == "RUNNING") return game::GameState::RUNNING;
    if (str == "PAUSED") return game::GameState::PAUSED;
    return game::GameState::RUNNING;
}

std::shared_ptr<GUIElement> GUIElement::fromJson(const json &data)
{
    auto element = std::make_shared<GUIElement>();

    element->id = data.value("id", "");
    element->type = stringToElementType(data.value("type", "PANEL"));
    element->anchor = stringToElementAnchor(data.value("anchor", "TOP_LEFT"));
    element->state = stringToElementState(data.value("state", "RUNNING"));

    if (
        data.contains("position") &&
        data["position"].is_array() &&
        data["position"].size() == 2
    ) {
        element->position.x = data["position"][0].get<float>();
        element->position.y = data["position"][1].get<float>();
    }

    if (
        data.contains("size") &&
        data["size"].is_array() &&
        data["size"].size() == 2
    ) {
        element->size.x = data["size"][0].get<float>();
        element->size.y = data["size"][1].get<float>();
    }

    element->visible = data.value("visible", true);
    element->texture = data.value("textureID", "");
    
    if (
        data.contains("uv") &&
        data["uv"].is_array() &&
        data["uv"].size() == 4
    ) {
        element->uv.x = data["uv"][0].get<float>();
        element->uv.y = data["uv"][1].get<float>();
        element->uv.z = data["uv"][2].get<float>();
        element->uv.w = data["uv"][3].get<float>();
    }

    element->invertEffect = data.value("invertEffect", false);

    return element;
}

json GUIElement::toJson() const
{
    json data;
    data["id"] = id;
    data["type"] = elementTypeToString(type);
    data["anchor"] = elementAnchorToString(anchor);
    data["position"] = { position.x, position.y };
    data["size"] = { size.x, size.y };
    data["visible"] = visible;
    data["state"] = elementStateToString(state);
    data["textureID"] = texture;
    data["uv"] = { uv.x, uv.y, uv.z, uv.w };
    data["invertEffect"] = invertEffect;

    return data;
}

} // namespace gui