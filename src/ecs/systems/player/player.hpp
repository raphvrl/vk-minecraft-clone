#pragma once

#include "ecs/systems/system.hpp"
#include "core/window/window.hpp"
#include "core/camera/camera.hpp"
#include "world/world.hpp"

#include "ecs/components/physics/transform.hpp"
#include "ecs/components/physics/velocity.hpp"
#include "ecs/components/player/player.hpp"

namespace sys
{

class Player : public ecs::System
{

public:
    Player(
        ecs::ECS *ecs,
        core::Window &window,
        core::Camera &camera,
        wld::World &world
    );

    void tick(f32 dt) override;

private:
    core::Window &m_window;
    core::Camera &m_camera;
    wld::World &m_world;

    EntityID m_playerEntity = ENTITY_NULL;
};

} // namespace sys