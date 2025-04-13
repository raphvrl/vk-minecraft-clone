#pragma once

#include <cmath>

#include "ecs/components/component.hpp"

namespace cmp
{
struct Player : public ecs::Component
{
    static constexpr f32 JUMP_HEIGHT = 1.5f;

    f32 moveSpeed = 4.3f;
    f32 jumpForce = std::sqrt(2.0f * std::abs(GRAVITY) * JUMP_HEIGHT);
    f32 sensitivity = 0.15f;

    f32 breakCooldown = 0.0f;
    f32 placeCooldown = 0.0f;

    bool isFlying = false;

    f32 eyeHeight = 1.8f;

    bool isInWater = false;
    f32 swimSpeed = 3.0f;
};

} // namespace ecs