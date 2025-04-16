#pragma once

#include "ecs/components/component.hpp"

#include <glm/glm.hpp>

namespace ecs
{

struct ColliderComponent : public ecs::Component
{
    glm::vec3 offset = glm::vec3(0.0f);
    glm::vec3 size = glm::vec3(1.0f);

    bool isGrounded = false;

    f32 groundOffset = 0.001f;

    bool isGhost = false;
};

} // namespace ecs