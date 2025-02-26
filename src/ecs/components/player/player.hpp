#pragma once

#include "ecs/components/component.hpp"

namespace cmp
{

struct Player : public ecs::Component
{
    f32 moveSpeed = 10.0f;
    f32 jumpForce = 8.0f;
    f32 sensitivity = 0.15f;

    f32 breakCooldown = 0.0f;
    f32 placeCooldown = 0.0f;

    bool isFlying = true;

    f32 eyeHeight = 1.8f;
};

} // namespace ecs