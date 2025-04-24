#include "light_manager.hpp"
#include "world/core/world.hpp"
#include "world/blocks/block_registry.hpp"

namespace wld
{

void LightManager::init(World &world)
{
    m_world = &world;
}

void LightManager::update()
{
    int nodesProcessed = 0;

    while (
        !m_blockLightQueue.empty() &&
        nodesProcessed < MAX_NODE_PROCESSING
    ) {
        propagateBlockLight();
        nodesProcessed++;
    }

    while (
        !m_blockLightRemoveQueue.empty() &&
        nodesProcessed < MAX_NODE_PROCESSING
    ) {
        propagateBlockLight();
        nodesProcessed++;
    }

    while (
        !m_skyLightQueue.empty() &&
        nodesProcessed < MAX_NODE_PROCESSING
    ) {
        propagateSkyLight();
        nodesProcessed++;
    }

    while (
        !m_skyLightRemoveQueue.empty() &&
        nodesProcessed < MAX_NODE_PROCESSING
    ) {
        propagateSkyLight();
        nodesProcessed++;
    }
}

void LightManager::onBlockPlaced(const glm::ivec3 &pos, BlockType type)
{
    if (!BlockRegistry::get().getBlock(type).transparency) {
        removeBlockLight(pos);
        removeSkyLight(pos);
    }

    u8 emission = BlockRegistry::get().getBlock(type).emission;
    if (emission > 0) {
        addBlockLight(pos, emission);
    }
}

void LightManager::onBlockRemoved(const glm::ivec3 &pos, BlockType oldType)
{
    u8 emission = BlockRegistry::get().getBlock(oldType).emission;
    if (emission > 0) {
        std::vector<LightNode> lightSources;
        for (const auto &dir : DIRECTIONS) {
            glm::ivec3 neighborPos = pos + dir;
            u8 neighborLevel = getBlockLightLevel(neighborPos);

            BlockType neighborType = m_world->getBlock(neighborPos);
            u8 neighborEmission = BlockRegistry::get().getBlock(neighborType).emission;

            if (neighborEmission > 0) {
                lightSources.push_back({ neighborPos, neighborEmission });
            }
        }

        removeBlockLight(pos);

        while (!m_blockLightRemoveQueue.empty()) {
            propagateBlockLight();
        }

        for (const auto &source : lightSources) {
            addBlockLight(source.pos, source.level);
        }
    }

    bool skyVisible = isSkyVisible(pos);
    
    if (skyVisible) {
        addSkyLight(pos, 15);
    } else {
        glm::ivec3 abovePos = pos + glm::ivec3(0, 1, 0);
        u8 skyLightAbove = getSkyLightLevel(abovePos);
        
        if (skyLightAbove > 0) {
            addSkyLight(pos, std::max(skyLightAbove - 1, 1));
        }

        for (const auto &dir : DIRECTIONS) {
            if (dir.y != 0) continue;
            
            glm::ivec3 neighborPos = pos + dir;
            u8 neighborSkyLevel = getSkyLightLevel(neighborPos);
            
            if (neighborSkyLevel > 1) {
                addSkyLight(pos, neighborSkyLevel - 1);
            }
        }
    }

    while (!m_skyLightRemoveQueue.empty()) {
        propagateSkyLight();
    }

    for (const auto &dir : DIRECTIONS) {
        glm::ivec3 neighborPos = pos + dir;
        u8 neighborLevel = getBlockLightLevel(neighborPos);

        if (neighborLevel > 1) {
            addBlockLight(pos, neighborLevel - 1);
        }
    }
}

void LightManager::addBlockLight(const glm::ivec3 &pos, u8 level)
{
    u8 currentLevel = getBlockLightLevel(pos);
    if (level <= currentLevel) {
        return;
    }

    setBlockLightLevel(pos, level);
    m_blockLightQueue.push({ pos, level });
}

void LightManager::removeBlockLight(const glm::ivec3 &pos)
{
    u8 level = getBlockLightLevel(pos);
    if (level == 0) {
        return;
    }

    setBlockLightLevel(pos, 0);
    m_blockLightRemoveQueue.push({ pos, level });
}

void LightManager::propagateBlockLight()
{
    if (!m_blockLightQueue.empty()) {
        LightNode node = m_blockLightQueue.front();
        m_blockLightQueue.pop();

        for (const auto &dir : DIRECTIONS) {
            glm::ivec3 neighborPos = node.pos + dir;
            if (isTransparent(neighborPos)) {
                u8 newLevel = node.level - 1;
                if (newLevel > 0 && newLevel > getBlockLightLevel(neighborPos)) {
                    setBlockLightLevel(neighborPos, newLevel);
                    m_blockLightQueue.push({ neighborPos, newLevel });
                }
            }
        }
    }

    if (!m_blockLightRemoveQueue.empty()) {
        LightNode node = m_blockLightRemoveQueue.front();
        m_blockLightRemoveQueue.pop();

        for (const auto &dir : DIRECTIONS) {
            glm::ivec3 neighborPos = node.pos + dir;
            u8 neighborLevel = getBlockLightLevel(neighborPos);

            if (neighborLevel == 0) {
                continue;
            }

            if (neighborLevel < node.level) {
                setBlockLightLevel(neighborPos, 0);
                m_blockLightRemoveQueue.push({ neighborPos, neighborLevel });
            } else {
                m_blockLightQueue.push({ neighborPos, neighborLevel });
            }
        }
    }
}

void LightManager::addSkyLight(const glm::ivec3 &pos, u8 level)
{
    u8 currentLevel = getSkyLightLevel(pos);
    if (level <= currentLevel) {
        return;
    }

    setSkyLightLevel(pos, level);
    m_skyLightQueue.push({ pos, level });
}

void LightManager::removeSkyLight(const glm::ivec3 &pos)
{
    u8 level = getSkyLightLevel(pos);
    if (level == 0) {
        return;
    }

    setSkyLightLevel(pos, 0);
    m_skyLightRemoveQueue.push({ pos, level });
}

void LightManager::propagateSkyLight()
{
    if (!m_skyLightQueue.empty()) {
        LightNode node = m_skyLightQueue.front();
        m_skyLightQueue.pop();

        for (const auto &dir : DIRECTIONS) {
            glm::ivec3 neighborPos = node.pos + dir;

            if (isTransparent(neighborPos)) {
                u8 newLevel;

                if (dir.y == -1) {
                    newLevel = node.level;
                } else {
                    newLevel = (node.level > 0) ? node.level - 1 : 0;
                }

                BlockType neighborBlock = m_world->getBlock(neighborPos);
                if (neighborBlock == BlockType::WATER) {
                    newLevel = std::max(newLevel - 2, 0);
                }

                if (newLevel > 0 && newLevel > getSkyLightLevel(neighborPos)) {
                    setSkyLightLevel(neighborPos, newLevel);
                    m_skyLightQueue.push({ neighborPos, newLevel });
                }
            }
        }
    }

    if (!m_skyLightRemoveQueue.empty()) {
        LightNode node = m_skyLightRemoveQueue.front();
        m_skyLightRemoveQueue.pop();

        for (const auto &dir : DIRECTIONS) {
            glm::ivec3 neighborPos = node.pos + dir;
            u8 neighborLevel = getSkyLightLevel(neighborPos);

            if (neighborLevel == 0) {
                continue;
            }

            bool shouldRemove = false;

            if (dir.y == -1) {
                shouldRemove = (neighborLevel <= node.level);
            } else {
                shouldRemove = (neighborLevel < node.level || 
                               (neighborLevel == node.level && neighborLevel < 15));
            }

            if (shouldRemove) {
                setSkyLightLevel(neighborPos, 0);
                m_skyLightRemoveQueue.push({ neighborPos, neighborLevel });
            } else {
                m_skyLightQueue.push({ neighborPos, neighborLevel });
            }
        }
    }
}

void LightManager::initalizeChunkLight(Chunk *chunk)
{
    auto &pos = chunk->getPos();

    for (int x = 0; x < Chunk::CHUNK_SIZE; x++) {
        for (int z = 0; z < Chunk::CHUNK_SIZE; z++) {
            bool skyVisible = true;
            for (int y = Chunk::CHUNK_HEIGHT - 1; y >= 0; y--) {
                if (skyVisible) {
                    BlockType block = chunk->getBlock(x, y, z);
                    if (
                        block != BlockType::AIR &&
                        !BlockRegistry::get().getBlock(block).transparency
                    ) {
                        skyVisible = false;
                    } else {
                        chunk->setSkyLight(x, y, z, 15);
                    }
                }
            }
        }
    }

    std::queue<LightNode> skyLightQueue;

    for (int x = 0; x < Chunk::CHUNK_SIZE; x++) {
        for (int z = 0; z < Chunk::CHUNK_SIZE; z++) {
            for (int y = 0; y < Chunk::CHUNK_HEIGHT; y++) {
                u8 skyLight = chunk->getSkyLight(x, y, z);
                if (skyLight > 0) {
                    glm::ivec3 worldPos(
                        pos.x * Chunk::CHUNK_SIZE + x,
                        y,
                        pos.z * Chunk::CHUNK_SIZE + z
                    );
                    skyLightQueue.push({ worldPos, skyLight });
                }
            }
        }
    }

    while (!skyLightQueue.empty()) {
        LightNode node = skyLightQueue.front();
        skyLightQueue.pop();
        
        for (const auto &dir : DIRECTIONS) {
            glm::ivec3 neighborPos = node.pos + dir;

            if (isTransparent(neighborPos)) {
                u8 newLevel;

                if (dir.y == -1) {
                    newLevel = node.level;
                } else {
                    newLevel = (node.level > 0) ? node.level - 1 : 0;
                }

                BlockType neighborBlock = m_world->getBlock(neighborPos);
                if (neighborBlock == BlockType::WATER) {
                    newLevel = std::max(newLevel - 2, 0);
                }

                if (newLevel > 0 && newLevel > getSkyLightLevel(neighborPos)) {
                    setSkyLightLevel(neighborPos, newLevel);
                    skyLightQueue.push({ neighborPos, newLevel });
                }
            }
        }
    }

    for (int x = 0; x < Chunk::CHUNK_SIZE; x++) {
        for (int z = 0; z < Chunk::CHUNK_SIZE; z++) {
            for (int y = 0; y < Chunk::CHUNK_HEIGHT; y++) {
                BlockType block = chunk->getBlock(x, y, z);
                u8 emission = BlockRegistry::get().getBlock(block).emission;

                if (emission > 0) {
                    glm::ivec3 worldPos(
                        pos.x * Chunk::CHUNK_SIZE + x,
                        y,
                        pos.z * Chunk::CHUNK_SIZE + z
                    );

                    addBlockLight(worldPos, emission);
                }
            }
        }
    }
}

bool LightManager::isTransparent(const glm::ivec3 &pos)
{
    if (pos.y < 0 || pos.y >= Chunk::CHUNK_HEIGHT) {
        return false;
    }

    BlockType block = m_world->getBlock(pos);
    return block == BlockType::AIR || BlockRegistry::get().getBlock(block).transparency;
}

u8 LightManager::getBlockLightLevel(const glm::ivec3 &pos)
{
    Chunk *chunk = getChunkAt(pos);
    if (!chunk) return 0;
    
    int localX = pos.x - (chunk->getPos().x * Chunk::CHUNK_SIZE);
    int localZ = pos.z - (chunk->getPos().z * Chunk::CHUNK_SIZE);
    
    return chunk->getBlockLight(localX, pos.y, localZ);
}

void LightManager::setBlockLightLevel(const glm::ivec3 &pos, u8 level)
{
    Chunk *chunk = getChunkAt(pos);
    if (!chunk) return;
    
    int localX = pos.x - (chunk->getPos().x * Chunk::CHUNK_SIZE);
    int localZ = pos.z - (chunk->getPos().z * Chunk::CHUNK_SIZE);
    
    chunk->setBlockLight(localX, pos.y, localZ, level);
}

u8 LightManager::getSkyLightLevel(const glm::ivec3 &pos)
{
    if (pos.y < 0 || pos.y >= Chunk::CHUNK_HEIGHT) {
        return 15;
    }

    Chunk *chunk = getChunkAt(pos);
    if (!chunk) return 15;
    
    int localX = pos.x - (chunk->getPos().x * Chunk::CHUNK_SIZE);
    int localZ = pos.z - (chunk->getPos().z * Chunk::CHUNK_SIZE);
    
    return chunk->getSkyLight(localX, pos.y, localZ);
}

void LightManager::setSkyLightLevel(const glm::ivec3 &pos, u8 level)
{
    Chunk *chunk = getChunkAt(pos);
    if (!chunk) return;
    
    int localX = pos.x - (chunk->getPos().x * Chunk::CHUNK_SIZE);
    int localZ = pos.z - (chunk->getPos().z * Chunk::CHUNK_SIZE);
    
    chunk->setSkyLight(localX, pos.y, localZ, level);
}

Chunk* LightManager::getChunkAt(const glm::ivec3 &pos)
{
    ChunkPos chunkPos(
        (pos.x < 0) ? (pos.x - (Chunk::CHUNK_SIZE - 1)) / Chunk::CHUNK_SIZE : pos.x / Chunk::CHUNK_SIZE,
        (pos.z < 0) ? (pos.z - (Chunk::CHUNK_SIZE - 1)) / Chunk::CHUNK_SIZE : pos.z / Chunk::CHUNK_SIZE
    );
    
    return m_world->getChunk(chunkPos);
}

bool LightManager::isSkyVisible(const glm::ivec3 &pos) 
{
    for (int y = pos.y + 1; y < Chunk::CHUNK_HEIGHT; y++) {
        glm::ivec3 checkPos(pos.x, y, pos.z);
        BlockType block = m_world->getBlock(checkPos);
        
        if (
            block != BlockType::AIR &&
            !BlockRegistry::get().getBlock(block).transparency) {
            return false;
        }
    }
    
    return true;
}

const std::array<glm::ivec3, 6> LightManager::DIRECTIONS = {
    glm::ivec3(1, 0, 0),
    glm::ivec3(-1, 0, 0),
    glm::ivec3(0, 0, 1),
    glm::ivec3(0, 0, -1),
    glm::ivec3(0, 1, 0),
    glm::ivec3(0, -1, 0)
};

} // namespace wld