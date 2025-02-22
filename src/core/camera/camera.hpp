#pragma once

#include <glm/ext.hpp>

#include "core/types.hpp"

namespace core
{

class Camera
{

public:
    Camera();
    virtual ~Camera() = default;

    Camera(const Camera &) = delete;
    Camera &operator=(const Camera &) = delete;

    void update();
    void updateProj(f32 aspect);

    void moveForward(f32 speed);
    void moveBackward(f32 speed);
    void moveRight(f32 speed);
    void moveLeft(f32 speed);
    void moveUp(f32 speed);
    void moveDown(f32 speed);

    glm::mat4 getView() { return m_view; }
    glm::mat4 getProj() { return m_proj; }

private:
    glm::vec3 m_pos;
    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::vec3 m_right;

    f32 m_yaw;
    f32 m_pitch;

    f32 m_fov;
    f32 m_aspect;
    f32 m_near;
    f32 m_far;

    glm::mat4 m_view;
    glm::mat4 m_proj;
};

} // namespace core