#pragma once

#include <array>

#include "core/types.hpp"
#include "world/block.hpp"

namespace wld
{

class World;
class BlockRegistry;

struct ChunkPos
{
    int x, z;

    ChunkPos() : x(0), z(0) {}

    ChunkPos(int x, int z) : x(x), z(z) {}

    ChunkPos(const ChunkPos &other) : x(other.x), z(other.z) {}

    bool operator==(const ChunkPos &other) const
    {
        return x == other.x && z == other.z;
    }

    bool operator!=(const ChunkPos &other) const
    {
        return x != other.x || z != other.z;
    }

    ChunkPos &operator=(const ChunkPos &other)
    {
        x = other.x;
        z = other.z;
        return *this;
    }
};

struct LightNode
{
    int x, y, z;
    u8 level;
};

class Chunk
{

public:
    static constexpr int CHUNK_SIZE = 16;
    static constexpr int CHUNK_HEIGHT = 128;

    Chunk(World &world, BlockRegistry &blockRegistry, const ChunkPos &pos);

    void update();
    void propagateLight();

    void setBlock(int x, int y, int z, BlockType type);
    void setBlock(glm::ivec3 &pos, BlockType type) {
        setBlock(pos.x, pos.y, pos.z, type);
    }

    void setLight(int x, int y, int z, u8 light);
    void setLight(glm::ivec3 &pos, u8 light) {
        setLight(pos.x, pos.y, pos.z, light);
    }

    BlockType getBlock(int x, int y, int z) const;

    u8 getLight(int x, int y, int z) const;

    
private:
    World &m_world;
    BlockRegistry &m_blockRegistry;
    ChunkPos m_pos;

    std::array<BlockType, CHUNK_SIZE * CHUNK_HEIGHT * CHUNK_SIZE> m_blocks;
    std::array<u8, CHUNK_SIZE * CHUNK_HEIGHT * CHUNK_SIZE> m_lights;

    int getIndex(int x, int y, int z) const {
        return y * CHUNK_SIZE * CHUNK_SIZE + z * CHUNK_SIZE + x;
    }

    void calulateSkyLight();
};

} // namespace wld