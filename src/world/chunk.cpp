#include "chunk.hpp"
#include "world.hpp"

namespace wld
{

Chunk::Chunk()
{
    m_blocks.fill(BlockType::AIR);
    m_lights.fill(15);
}

void Chunk::update()
{
    m_lights.fill(0);
    calulateSkyLight();
    propagateLight();
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

void Chunk::calulateSkyLight()
{
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
            u8 currentLight = 15;
            for (int y = CHUNK_HEIGHT - 1; y >= 0; y--) {
                BlockType block = getBlock(x, y, z);

                if (block != BlockType::AIR && block != BlockType::WATER) {
                    setLight(x, y, z, 0);
                    currentLight = 0;
                } else {
                    setLight(x, y, z, currentLight);

                    if (block == BlockType::WATER) {
                        currentLight = std::max(currentLight - 3, 0);
                    }
                }
            }
        }
    }
}

void Chunk::propagateLight()
{
    std::queue<LightNode> lightQueue;

    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
            for (int y = 0; y < CHUNK_HEIGHT; y++) {
                u8 lightLevel = getLight(x, y, z);
                if (lightLevel > 0) {
                    lightQueue.push({x, y, z, lightLevel});
                }
            }
        }
    }

    const std::array<glm::ivec3, 6> directions = {
        glm::ivec3(1, 0, 0),
        glm::ivec3(-1, 0, 0),
        glm::ivec3(0, 1, 0),
        glm::ivec3(0, -1, 0),
        glm::ivec3(0, 0, 1),
        glm::ivec3(0, 0, -1)
    };

    while (!lightQueue.empty()) {
        LightNode node = lightQueue.front();
        lightQueue.pop();

        for (const auto &dir : directions) {
            glm::ivec3 adjPos = glm::ivec3(node.x, node.y, node.z) + dir;

            bool isInChunk = adjPos.x >= 0 && adjPos.x < CHUNK_SIZE &&
                             adjPos.y >= 0 && adjPos.y < CHUNK_HEIGHT &&
                             adjPos.z >= 0 && adjPos.z < CHUNK_SIZE;

            if (isInChunk) {
                BlockType neighborBlock = getBlock(adjPos.x, adjPos.y, adjPos.z);
                if (
                    neighborBlock != BlockType::AIR &&
                    neighborBlock != BlockType::WATER
                ) {
                    continue;
                }

                u8 currentLight = getLight(adjPos.x, adjPos.y, adjPos.z);
                u8 propagatedLight = std::max(node.level - 1, 0);

                if (neighborBlock == BlockType::WATER) {
                    propagatedLight = std::max(propagatedLight - 3, 0);
                }

                if (propagatedLight > currentLight) {
                    setLight(adjPos.x, adjPos.y, adjPos.z, propagatedLight);
                    lightQueue.push({adjPos.x, adjPos.y, adjPos.z, propagatedLight});
                }
            }
        }
    }
}

} // namespace wld