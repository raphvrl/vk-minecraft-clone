#pragma once

#include <array>

#include "core/types.hpp"
#include "world/block.hpp"

namespace wld
{

struct ChunkPos;

class Chunk
{

public:
    static constexpr int CHUNK_SIZE = 16;
    static constexpr int CHUNK_HEIGHT = 128;

    Chunk();

    void setBlock(int x, int y, int z, BlockType type);
    void setBlock(glm::ivec3 pos, BlockType type) {
        setBlock(pos.x, pos.y, pos.z, type);
    }

    BlockType getBlock(int x, int y, int z) const;

    
private:
    std::array<BlockType, CHUNK_SIZE * CHUNK_HEIGHT * CHUNK_SIZE> m_blocks;

    int getIndex(int x, int y, int z) const {
        return y * CHUNK_SIZE * CHUNK_SIZE + z * CHUNK_SIZE + x;
    }
};

} // namespace wld