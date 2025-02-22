#include "camera.hpp"

namespace core
{

Camera::Camera()
{
    m_pos = glm::vec3(0.0f, 0.0f, 3.0f);
    m_front = glm::vec3(0.0f, 0.0f, -1.0f);
    m_up = glm::vec3(0.0f, 1.0f, 0.0f);
    m_right = glm::vec3(1.0f, 0.0f, 0.0f);

    m_yaw = -90.0f;
    m_pitch = 0.0f;

    m_fov = 45.0f;
    m_aspect = 1280.0f / 720.0f;
    m_near = 0.1f;
    m_far = 100.0f;

    m_view = glm::lookAt(m_pos, m_pos + m_front, m_up);
    m_proj = glm::perspective(glm::radians(m_fov), m_aspect, m_near, m_far);
}

void Camera::update()
{
    glm::vec3 front;
    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_front = glm::normalize(front);

    m_right = glm::normalize(glm::cross(m_front, m_up));
    m_up = glm::normalize(glm::cross(m_right, m_front));

    m_view = glm::lookAt(m_pos, m_pos + m_front, m_up);
}

void Camera::moveForward(f32 speed)
{
    m_pos += m_front * speed;
}

void Camera::moveBackward(f32 speed)
{
    m_pos -= m_front * speed;
}

void Camera::moveRight(f32 speed)
{
    m_pos += m_right * speed;
}

void Camera::moveLeft(f32 speed)
{
    m_pos -= m_right * speed;
}

void Camera::moveUp(f32 speed)
{
    m_pos += m_up * speed;
}

void Camera::moveDown(f32 speed)
{
    m_pos -= m_up * speed;
}

} // namespace core