#pragma once

#include <glm/glm.hpp>

#include <queue>
#include <array>
#include <vector>
#include <algorithm>

#include "core/types.hpp"
#include "world/blocks/block.hpp"
#include "world/chunk/chunk.hpp"

namespace wld
{

class World;

struct LightNode
{
    glm::ivec3 pos;
    u8 level;
};

class LightManager
{

public:
    void init(World &world);

    void update();

    void onBlockPlaced(const glm::ivec3 &pos, BlockType type);
    void onBlockRemoved(const glm::ivec3 &pos, BlockType oldType);

    void initalizeChunkLight(Chunk *chunk);

private:
    World *m_world = nullptr;

    std::queue<LightNode> m_blockLightQueue;
    std::queue<LightNode> m_blockLightRemoveQueue;
    std::queue<LightNode> m_skyLightQueue;
    std::queue<LightNode> m_skyLightRemoveQueue;

    void addBlockLight(const glm::ivec3 &pos, u8 level);
    void removeBlockLight(const glm::ivec3 &pos);
    void propagateBlockLight();

    void addSkyLight(const glm::ivec3 &pos, u8 level);
    void removeSkyLight(const glm::ivec3 &pos);
    void propagateSkyLight();
    
    bool isTransparent(const glm::ivec3 &pos);
    u8 getBlockLightLevel(const glm::ivec3 &pos);
    void setBlockLightLevel(const glm::ivec3 &pos, u8 level);
    u8 getSkyLightLevel(const glm::ivec3 &pos);
    void setSkyLightLevel(const glm::ivec3 &pos, u8 level);
    Chunk *getChunkAt(const glm::ivec3 &pos);
    bool isSkyVisible(const glm::ivec3 &pos);

    static const std::array<glm::ivec3, 6> DIRECTIONS;
    static const int MAX_NODE_PROCESSING = 1000;
};

} // namespace wld