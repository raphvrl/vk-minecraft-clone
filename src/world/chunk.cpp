#include "chunk.hpp"
#include "world.hpp"

namespace wld
{

Chunk::Chunk()
{
    m_blocks.fill(BlockType::AIR);
    m_lights.fill(15);
    calculateLight();
}

void Chunk::update()
{
    calculateLight();
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

void Chunk::setLight(int x, int y, int z, u8 light)
{
    if (
        x < 0 || x >= CHUNK_SIZE ||
        y < 0 || y >= CHUNK_HEIGHT ||
        z < 0 || z >= CHUNK_SIZE
    ) {
        return;
    }

    m_lights[getIndex(x, y, z)] = light;
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

    return m_lights[getIndex(x, y, z)];
}

void Chunk::calculateLight()
{
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
            u8 currentLight = 15;
            for (int y = CHUNK_HEIGHT - 1; y >= 0; y--) {
                BlockType block = getBlock(x, y, z);

                setLight(x, y, z, currentLight);

                if (block == BlockType::WATER) {
                    currentLight = std::max(currentLight - 3, 1);
                } else if (block != BlockType::AIR) {
                    currentLight = 0;
                }
            }
        }
    }
}

} // namespace wld