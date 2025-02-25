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

    void rotate(f32 xoffset, f32 yoffset);

    glm::vec3 getPos() const { return m_pos; }
    glm::vec3 getFront() const { return m_front; }
    glm::vec3 getUp() const { return m_up; }
    glm::vec3 getRight() const { return m_right; }

    glm::mat4 getView() const { return m_view; }
    glm::mat4 getProj() const { return m_proj; }

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