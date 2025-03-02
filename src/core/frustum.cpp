#include "frustum.hpp"

namespace core
{

enum PlaneIndex
{
    LEFT,
    RIGHT,
    TOP,
    BOTTOM,
    NEAR,
    FAR
};

Frustum Frustum::fromViewProj(const glm::mat4 &view, const glm::mat4 &proj)
{
    Frustum frustum;
    glm::mat4 viewProj = proj * view;

    frustum.m_planes[LEFT] = glm::vec4(
        viewProj[0][3] + viewProj[0][0],
        viewProj[1][3] + viewProj[1][0],
        viewProj[2][3] + viewProj[2][0],
        viewProj[3][3] + viewProj[3][0]
    );

    frustum.m_planes[RIGHT] = glm::vec4(
        viewProj[0][3] - viewProj[0][0],
        viewProj[1][3] - viewProj[1][0],
        viewProj[2][3] - viewProj[2][0],
        viewProj[3][3] - viewProj[3][0]
    );

    frustum.m_planes[TOP] = glm::vec4(
        viewProj[0][3] - viewProj[0][1],
        viewProj[1][3] - viewProj[1][1],
        viewProj[2][3] - viewProj[2][1],
        viewProj[3][3] - viewProj[3][1]
    );

    frustum.m_planes[BOTTOM] = glm::vec4(
        viewProj[0][3] + viewProj[0][1],
        viewProj[1][3] + viewProj[1][1],
        viewProj[2][3] + viewProj[2][1],
        viewProj[3][3] + viewProj[3][1]
    );

    frustum.m_planes[NEAR] = glm::vec4(
        viewProj[0][3] + viewProj[0][2],
        viewProj[1][3] + viewProj[1][2],
        viewProj[2][3] + viewProj[2][2],
        viewProj[3][3] + viewProj[3][2]
    );

    frustum.m_planes[FAR] = glm::vec4(
        viewProj[0][3] - viewProj[0][2],
        viewProj[1][3] - viewProj[1][2],
        viewProj[2][3] - viewProj[2][2],
        viewProj[3][3] - viewProj[3][2]
    );

    frustum.normalizePlane();
    return frustum;
}

bool Frustum::isBoxVisible(const glm::vec3 &min, const glm::vec3 &max) const
{
    for (const auto &plane: m_planes) {
        glm::vec3 p(
            (plane.x < 0) ? min.x : max.x,
            (plane.y < 0) ? min.y : max.y,
            (plane.z < 0) ? min.z : max.z
        );

        if (glm::dot(glm::vec3(plane), p) + plane.w < 0) {
            return false;
        }
    }

    return true;
}

void Frustum::normalizePlane()
{
    for (auto &plane : m_planes) {
        f32 length = glm::length(glm::vec3(plane));
        plane /= length;
    }
}

} // namespace core