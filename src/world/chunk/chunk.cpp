#include "chunk.hpp"
#include "world/core/world.hpp"
#include "world/blocks/block_registry.hpp"

namespace wld
{

Chunk::Chunk(World &world, const ChunkPos &pos) :
    m_world(world),
    m_pos(pos)
{
    m_blocks.fill(BlockType::AIR);

    m_blockLights.fill(0);
    m_skyLights.fill(0);
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

    markDirty();
}

void Chunk::setBlockLight(int x, int y, int z, u8 light)
{
    if (
        x < 0 || x >= CHUNK_SIZE ||
        y < 0 || y >= CHUNK_HEIGHT ||
        z < 0 || z >= CHUNK_SIZE
    ) {
        return;
    }

    m_blockLights[getIndex(x, y, z)] = light;

    markDirty();
}

void Chunk::setSkyLight(int x, int y, int z, u8 light)
{
    if (
        x < 0 || x >= CHUNK_SIZE ||
        y < 0 || y >= CHUNK_HEIGHT ||
        z < 0 || z >= CHUNK_SIZE
    ) {
        return;
    }

    m_skyLights[getIndex(x, y, z)] = light;

    markDirty();
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

u8 Chunk::getLight(int x, int y, int z) const
{
    if (
        x < 0 || x >= CHUNK_SIZE ||
        y < 0 || y >= CHUNK_HEIGHT ||
        z < 0 || z >= CHUNK_SIZE
    ) {
        return 0;
    }

    return std::max(
        m_blockLights[getIndex(x, y, z)],
        m_skyLights[getIndex(x, y, z)]
    );
}

} // namespace wld