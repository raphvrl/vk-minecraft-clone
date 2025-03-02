#pragma once

#include <array>
#include <glm/ext.hpp>

#include "core/types.hpp"

namespace core
{

class Frustum
{

public:
    static Frustum fromViewProj(const glm::mat4 &view, const glm::mat4 &proj);

    bool isBoxVisible(const glm::vec3 &min, const glm::vec3 &max) const;

    const std::array<glm::vec4, 6> &getPlanes() const { return m_planes; }

private:
    std::array<glm::vec4, 6> m_planes;

    void normalizePlane();

};

} // namespace core