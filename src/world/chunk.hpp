#pragma once

#include <array>

#include "core/types.hpp"
#include "world/block.hpp"

namespace wld
{

struct ChunkPos;

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

    Chunk();

    void update();

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
    std::array<BlockType, CHUNK_SIZE * CHUNK_HEIGHT * CHUNK_SIZE> m_blocks;
    std::array<u8, CHUNK_SIZE * CHUNK_HEIGHT * CHUNK_SIZE> m_lights;

    int getIndex(int x, int y, int z) const {
        return y * CHUNK_SIZE * CHUNK_SIZE + z * CHUNK_SIZE + x;
    }

    void calculateLight();
    void calulateSkyLight();
    void propagateLight();
};

} // namespace wld