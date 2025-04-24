#pragma once

#include <array>

#include "core/types.hpp"
#include "world/blocks/block.hpp"

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

struct ChunkPosHash
{
    std::size_t operator()(const ChunkPos &pos) const {
        return std::hash<int>()(pos.x) ^ (std::hash<int>()(pos.z) << 1);
    }
};

class Chunk
{

public:
    static constexpr int CHUNK_SIZE = 16;
    static constexpr int CHUNK_HEIGHT = 128;

    Chunk(World &world, const ChunkPos &pos);

    void setBlock(int x, int y, int z, BlockType type);
    void setBlock(glm::ivec3 &pos, BlockType type) {
        setBlock(pos.x, pos.y, pos.z, type);
    }

    void setBlockLight(int x, int y, int z, u8 light);
    void setBlockLight(glm::ivec3 &pos, u8 light) {
        setBlockLight(pos.x, pos.y, pos.z, light);
    }

    u8 getBlockLight(int x, int y, int z) const {
        return m_blockLights[getIndex(x, y, z)];
    }

    u8 getBlockLight(glm::ivec3 &pos) const {
        return getBlockLight(pos.x, pos.y, pos.z);
    }

    void setSkyLight(int x, int y, int z, u8 light);
    void setSkyLight(glm::ivec3 &pos, u8 light) {
        setSkyLight(pos.x, pos.y, pos.z, light);
    }

    u8 getSkyLight(int x, int y, int z) const {
        return m_skyLights[getIndex(x, y, z)];
    }

    const ChunkPos &getPos() const { return m_pos; }

    BlockType getBlock(const glm::ivec3 &pos) const {
        return getBlock(pos.x, pos.y, pos.z);
    }
    BlockType getBlock(int x, int y, int z) const;
    u8 getLight(int x, int y, int z) const;

    bool isDirty() const { return m_isDirty; }
    void markDirty() { m_isDirty = true; }
    void clearDirty() { m_isDirty = false; }

    
private:
    World &m_world;
    ChunkPos m_pos;

    std::array<BlockType, CHUNK_SIZE * CHUNK_HEIGHT * CHUNK_SIZE> m_blocks;
    std::array<u8, CHUNK_SIZE * CHUNK_HEIGHT * CHUNK_SIZE> m_blockLights;
    std::array<u8, CHUNK_SIZE * CHUNK_HEIGHT * CHUNK_SIZE> m_skyLights;

    int getIndex(int x, int y, int z) const {
        return y * CHUNK_SIZE * CHUNK_SIZE + z * CHUNK_SIZE + x;
    }

    bool m_isDirty;
};

} // namespace wld