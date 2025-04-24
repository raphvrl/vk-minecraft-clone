#pragma once

#include <glm/glm.hpp>

#include "core/types.hpp"

struct ivec2Hash {
    usize operator()(const glm::ivec2 &v) const {
        return std::hash<int>()(v.x) ^ std::hash<int>()(v.y);
    }
};

struct ivec3Hash {
    usize operator()(const glm::ivec3 &v) const {
        return std::hash<int>()(v.x) ^ std::hash<int>()(v.y) ^ std::hash<int>()(v.z);
    }
};

struct ivec4Hash {
    usize operator()(const glm::ivec4 &v) const {
        return std::hash<int>()(v.x) ^ std::hash<int>()(v.y) ^ std::hash<int>()(v.z) ^ std::hash<int>()(v.w);
    }
};