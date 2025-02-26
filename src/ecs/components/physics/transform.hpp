#pragma once

#include "ecs/components/component.hpp"

#include <glm/glm.hpp>

namespace cmp
{

struct Transform : public ecs::Component
{
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
};

} // namespace ecs