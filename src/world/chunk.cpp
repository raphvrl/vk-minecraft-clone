#include "chunk.hpp"

namespace wld
{

Chunk::Chunk()
{
    m_blocks.fill(BlockType::AIR);
}

void Chunk::setBlock(int x, int y, int z, BlockType type)
{
    if (
        x < 0 || x >= CHUNK_SIZE ||
        y < 0 || y >= CHUNK_HEIGHT ||
        z < 0 || z >= CHUNK_SIZE
    ) {
        return;
    }

    m_blocks[getIndex(x, y, z)] = type;
}

BlockType Chunk::getBlock(int x, int y, int z) const
{
    if (
        x < 0 || x >= CHUNK_SIZE ||
        y < 0 || y >= CHUNK_HEIGHT ||
        z < 0 || z >= CHUNK_SIZE
    ) {
        return BlockType::AIR;
    }

    return m_blocks[getIndex(x, y, z)];
}


} // namespace wld