#pragma once

#include "ecs/components/component.hpp"

#include <glm/glm.hpp>

namespace cmp
{

struct Velocity : public ecs::Component
{
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 acceleration = glm::vec3(0.0f);
};

} // namespace ecs