#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <unordered_set>
#include <algorithm>
#include <thread>
#include <mutex>
#include <queue>
#include <future>

#include "chunk.hpp"
#include "chunk_mesh.hpp"
#include "block.hpp"
#include "block_registry.hpp"
#include "world_generator.hpp"
#include "core/camera/camera.hpp"
#include "graphics/device.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/texture_cache.hpp"
#include "core/frustum.hpp"

namespace wld
{

struct Ray
{
    glm::vec3 origin;
    glm::vec3 direction;
};

struct RaycastResult
{
    glm::ivec3 pos;
    glm::ivec3 normal;
    Face face;
};

class World
{

public:
    void init(gfx::Device &device, gfx::TextureCache &textureCache);
    void destroy();

    void update(const glm::vec3 &playerPos, f32 dt);
    void render(const core::Camera &camera, VkCommandBuffer cmd);

    BlockType getBlock(int x, int y, int z) const;
    BlockType getBlock(const glm::ivec3 &pos) const {
        return getBlock(pos.x, pos.y, pos.z);
    }
    
    void placeBlock(const glm::ivec3 &pos, BlockType type);
    void deleteBlock(const glm::ivec3 &pos);

    bool raycast(const Ray &ray, f32 maxDistance, RaycastResult &result);
    bool checkCollision(const glm::vec3 &min, const glm::vec3 &max);

    usize getUpdatedChunks() const { return m_updatedChunks; }

public:
    Chunk *getChunk(const ChunkPos &pos) const;


private:
    struct ChunkPosHash
    {
        std::size_t operator()(const ChunkPos &pos) const {
            return std::hash<int>()(pos.x) ^ (std::hash<int>()(pos.z) << 1);
        }
    };
    
    std::unordered_set<ChunkPos, ChunkPosHash> m_chunksNeeded;
    std::vector<std::pair<ChunkPos, f32>> m_chunksToLoad;
    std::vector<ChunkPos> m_chunksToUnload;

    void loadChunks(const ChunkPos &pos);
    void unloadChunks(const ChunkPos &pos);
    bool isChunkLoaded(const ChunkPos &pos);

    void updateMeshe(const ChunkPos &pos);

    static constexpr int RENDER_DISTANCE = 8;
    static constexpr int CHUNKS_PER_TICK = 1;

    std::queue<ChunkPos> m_pendingChunks;
    std::queue<ChunkPos> m_pendingMeshes;

    usize m_updatedChunks = 0;

    gfx::Device *m_device;
    BlockRegistry m_blockRegistry;

    enum PipelineType
    {
        P_OPAQUE,
        P_TRANSPARENT
    };

    std::array<gfx::Pipeline, 2> m_pipelines;

    u32 m_textureID;

    struct PushConstants
    {
        alignas(16) glm::mat4 model;
        alignas(4) u32 textureID;
    };

    core::Frustum m_frustum;

    using ChunkMap = std::unordered_map<ChunkPos,
        std::unique_ptr<Chunk>, 
        ChunkPosHash>;
    using ChunkMeshMap = std::unordered_map<ChunkPos, 
        std::unique_ptr<ChunkMesh>, 
        ChunkPosHash>;

    ChunkPos m_playerChunkPos;
    ChunkMap m_chunks;
    ChunkMeshMap m_meshes;

    WorldGenerator m_generator;
};

} // namespace wld