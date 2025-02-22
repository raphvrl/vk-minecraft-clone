#pragma once

#include <glm/glm.hpp>

#include <string>

#include "core/types.hpp"
#include "chunk_mesh.hpp"

namespace wld
{

struct TextureInfo
{
    glm::uvec2 north;
    glm::uvec2 south;
    glm::uvec2 east;
    glm::uvec2 west;
    glm::uvec2 top;
    glm::uvec2 bottom;

    void fill(const glm::uvec2 &uv)
    {
        north = south = east = west = top = bottom = uv;
    }

    void sides(const glm::uvec2 &uv)
    {
        north = south = east = west = uv;
    }

    glm::uvec2 getUV(Face face) const
    {
        switch (face)
        {
        case Face::NORTH:
            return north;
        case Face::SOUTH:
            return south;
        case Face::EAST:
            return east;
        case Face::WEST:
            return west;
        case Face::TOP:
            return top;
        case Face::BOTTOM:
            return bottom;
        default:
            return {0, 0};
        }
    }
};

class Block
{

public:
    Block() = default;

    const std::string &getName() const { return m_name; }
    const TextureInfo &getTextureInfo() const { return m_textureInfo; }

    void setName(const std::string &name) { m_name = name; }
    void setTextureInfo(const TextureInfo &textureInfo) { 
        m_textureInfo = textureInfo; 
    }

private:
    std::string m_name;
    TextureInfo m_textureInfo;

};

} // namespace wld