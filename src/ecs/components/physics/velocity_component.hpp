#pragma once

#include "ecs/components/component.hpp"

#include <glm/glm.hpp>

namespace ecs
{

struct VelocityComponent : public ecs::Component
{
    glm::vec3 position = glm::vec3(0.0f);
};

} // namespace ecs