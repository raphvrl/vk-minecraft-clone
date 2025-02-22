#pragma once

#include <array>

#include "core/types.hpp"

namespace wld
{

enum class BlockType
{
    AIR,
    STONE = 1,
    GRASS = 2,
    DIRT = 3,
    COBBLESTONE = 4
};

class Chunk
{

public:
    static constexpr int CHUNK_SIZE = 16;
    static constexpr int CHUNK_HEIGHT = 128;

    Chunk();

    void setBlock(int x, int y, int z, BlockType type);
    BlockType getBlock(int x, int y, int z) const;
    
private:
    std::array<BlockType, CHUNK_SIZE * CHUNK_HEIGHT * CHUNK_SIZE> m_blocks;

    int getIndex(int x, int y, int z) const {
        return y * CHUNK_SIZE * CHUNK_SIZE + z * CHUNK_SIZE + x;
    }
};

} // namespace wld