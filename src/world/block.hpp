
#pragma once

#include <glm/glm.hpp>

#include <string>

#include "core/types.hpp"
#include "chunk_mesh.hpp"

namespace wld
{

enum class BlockType
{
    AIR,
    STONE = 1,
    GRASS = 2,
    DIRT = 3,
    COBBLESTONE = 4,
    BEDROCK = 7,
    SAND = 12,
    WATER = 17,
};

struct TextureInfo
{
    std::array<glm::uvec2, 6> faces;

    void fill(const glm::uvec2 &uv)
    {
        faces.fill(uv);
    }

    void sides(const glm::uvec2 &uv)
    {
        faces[static_cast<u32>(Face::NORTH)] = uv;
        faces[static_cast<u32>(Face::SOUTH)] = uv;
        faces[static_cast<u32>(Face::EAST)] = uv;
        faces[static_cast<u32>(Face::WEST)] = uv;
    }

    void set(Face face, const glm::uvec2 &uv)
    {
        faces[static_cast<u32>(face)] = uv;
    }

    glm::uvec2 getUV(Face face) const
    {
        return faces[static_cast<u32>(face)];
    }
};

struct Block
{
    std::string name;
    TextureInfo textures;
    bool transparency = false;
};

} // namespace wld