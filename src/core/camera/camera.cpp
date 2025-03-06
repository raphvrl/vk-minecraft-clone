#include "camera.hpp"

namespace core
{

Camera::Camera()
{
    m_pos = glm::vec3(0.0f, 80.0f, 0.0f);
    m_front = glm::vec3(0.0f, 0.0f, -1.0f);
    m_up = glm::vec3(0.0f, 1.0f, 0.0f);
    m_right = glm::vec3(1.0f, 0.0f, 0.0f);

    m_yaw = 90.0f;
    m_pitch = 0.0f;

    m_fov = 63.0f;
    m_aspect = 1280.0f / 720.0f;
    m_near = 0.01f;
    m_far = 1000.0f;

    m_view = glm::lookAt(m_pos, m_pos + m_front, m_up);
    m_proj = glm::perspective(glm::radians(m_fov), m_aspect, m_near, m_far);

    m_proj[1][1] *= -1;
}

void Camera::updateView()
{
    glm::vec3 front;
    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_front = glm::normalize(front);

    m_right = glm::normalize(glm::cross(m_front, m_up));

    m_view = glm::lookAt(m_pos, m_pos + m_front, m_up);
}

void Camera::updateProj(f32 aspect)
{
    m_aspect = aspect;
    m_proj = glm::perspective(glm::radians(m_fov), m_aspect, m_near, m_far);

    m_proj[1][1] *= -1;
}

void Camera::rotate(f32 xoffset, f32 yoffset)
{
    m_yaw += xoffset;
    m_pitch += yoffset;

    m_pitch = glm::clamp(m_pitch, -89.0f, 89.0f);
}

} // namespace core