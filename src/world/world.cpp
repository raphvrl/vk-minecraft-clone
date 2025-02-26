#include "world.hpp"

namespace wld
{

void World::init(gfx::VulkanCtx &ctx)
{
    m_ctx = &ctx;
    m_blockRegistry.load("assets/config/blocks.toml");

    m_playerChunkPos = {-1, -1};

    auto binding = ChunkMesh::Vertex::getBindingDescription();
    auto attributes = ChunkMesh::Vertex::getAttributeDescriptions();

    m_pipeline = gfx::Pipeline::Builder(*m_ctx)
        .setShader(VK_SHADER_STAGE_VERTEX_BIT, "chunk.vert.spv")
        .setShader(VK_SHADER_STAGE_FRAGMENT_BIT, "chunk.frag.spv")
        .setVertexInput(
            &binding,
            attributes.data(),
            attributes.size()
        )
        .addDescriptorBinding({
            .binding = 0,
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .count = 1,
            .stage = VK_SHADER_STAGE_VERTEX_BIT
        })
        .addDescriptorBinding({
            .binding = 1,
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .count = 1,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT
        })
        .addPushConstant(
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(glm::mat4)
        )
        .build();

    m_texture.init(*m_ctx, "blocks.png");
    m_ubo.init(*m_ctx, sizeof(UniformBufferObject));

    std::vector<gfx::DescriptorData> descriptors = {
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .binding = 0,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .ubo = &m_ubo
        },
        {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .binding = 1,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .texture = &m_texture
        }
    };

    m_descriptorSet = m_pipeline.createDescriptorSet(descriptors);

    m_noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    m_noise.SetSeed(42);
    m_noise.SetFrequency(0.02f);
    m_noise.SetFractalOctaves(1);

    m_mountainNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    m_mountainNoise.SetSeed(42);
    m_mountainNoise.SetFrequency(0.015f);

    m_detailNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    m_detailNoise.SetSeed(42);
    m_detailNoise.SetFrequency(0.08f);

    m_chunks.reserve(RENDER_DISTANCE * RENDER_DISTANCE);
    m_meshes.reserve(RENDER_DISTANCE * RENDER_DISTANCE);

    update({0.0f, 0.0f, 0.0f});
}

void World::destroy()
{
    m_ubo.destroy();
    m_texture.destroy();
    m_pipeline.destroy();

    for (auto &[pos, mesh] : m_meshes) {
        mesh->destroy();
    }

    m_chunks.clear();
    m_meshes.clear();
}

void World::update(const glm::vec3 &playerPos)
{
    ChunkPos newPos = {
        static_cast<i32>(playerPos.x) / Chunk::CHUNK_SIZE,
        static_cast<i32>(playerPos.z) / Chunk::CHUNK_SIZE
    };

    if (newPos.x == m_playerChunkPos.x && newPos.z == m_playerChunkPos.z) {
        return;
    }

    std::unordered_set<ChunkPos, ChunkPosHash> chunksNeeded;
    std::vector<std::pair<ChunkPos, f32>> chunksToLoad;
    std::vector<ChunkPos> chunksToUnload;

    for (i32 x = -RENDER_DISTANCE; x <= RENDER_DISTANCE; x++) {
        for (i32 z = -RENDER_DISTANCE; z <= RENDER_DISTANCE; z++) {
            ChunkPos pos = {newPos.x + x, newPos.z + z};
            f32 dist = glm::distance(
                glm::vec2(newPos.x, newPos.z),
                glm::vec2(pos.x, pos.z)
            );

            if (dist <= RENDER_DISTANCE) {
                chunksNeeded.insert(pos);
                if (!isChunkLoaded(pos)) {
                    chunksToLoad.push_back({pos, dist});
                }
            }
        }
    }

    std::sort(chunksToLoad.begin(), chunksToLoad.end(), 
        [](const auto &a, const auto &b) {
            return a.second < b.second;
        }
    );

    for (const auto &[pos, _] : m_chunks) {
        if (chunksNeeded.find(pos) == chunksNeeded.end()) {
            chunksToUnload.push_back(pos);
        }
    }

    for (const auto &pos : chunksToUnload) {
        unloadChunks(pos);
    }

    usize chunkToLoadThisFrame = std::min(
        chunksToLoad.size(),
        static_cast<usize>(CHUNKS_PER_FRAME)
    );

    std::vector<ChunkPos> meshesToUpdate;

    for (usize i = 0; i < chunkToLoadThisFrame; i++) {
        const auto &pos = chunksToLoad[i].first;
        loadChunks(pos);
        meshesToUpdate.push_back(pos);

        for (i32 dx = -1; dx <= 1; dx++) {
            for (i32 dz = -1; dz <= 1; dz++) {
                if (dx == 0 && dz == 0) {
                    continue;
                }

                ChunkPos neighbor = {pos.x + dx, pos.z + dz};
                if (isChunkLoaded(neighbor)) {
                    meshesToUpdate.push_back(neighbor);
                }
            }
        }
    }

    for (const auto &pos : meshesToUpdate) {
        if (auto chunk = m_chunks.find(pos); chunk != m_chunks.end()) {
            if (auto it = m_meshes.find(pos); it != m_meshes.end()) {
                it->second->destroy();
                m_meshes.erase(it);
            }

            auto mesh = std::make_unique<ChunkMesh>();
            mesh->init(*m_ctx, m_blockRegistry);

            std::array<const Chunk *, 4> neighbors = {
                getChunk({pos.x - 1, pos.z}),
                getChunk({pos.x + 1, pos.z}),
                getChunk({pos.x, pos.z - 1}),
                getChunk({pos.x, pos.z + 1})
            };

            mesh->generate(*chunk->second, neighbors);
            m_meshes[pos] = std::move(mesh);
        }
    }

    if (chunksToLoad.size() <= CHUNKS_PER_FRAME) {
        m_playerChunkPos = newPos;
    }
}

void World::render(const core::Camera &camera)
{
    m_pipeline.bind();

    m_pipeline.bindDescriptorSet(m_descriptorSet);

    UniformBufferObject uniformBuffer = {
        .view = camera.getView(),
        .proj = camera.getProj(),
        .camPos = camera.getPos()
    };

    m_ubo.update(&uniformBuffer, sizeof(UniformBufferObject));

    for (const auto &[pos, mesh] : m_meshes) {
        f32 x = static_cast<f32>(pos.x * Chunk::CHUNK_SIZE);
        f32 z = static_cast<f32>(pos.z * Chunk::CHUNK_SIZE);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, {x, 0.0f, z});

        m_pipeline.push(
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(model),
            &model
        );

        mesh->draw();
    }
}

BlockType World::getBlock(int x, int y, int z) const
{
    if (y < 0 || y >= Chunk::CHUNK_HEIGHT) {
        return BlockType::AIR;
    }

    ChunkPos chunkPos = {
        .x = (x < 0) ? 
            (x - (Chunk::CHUNK_SIZE - 1)) / Chunk::CHUNK_SIZE :
                x / Chunk::CHUNK_SIZE,
        .z = (z < 0) ? 
            (z - (Chunk::CHUNK_SIZE - 1)) / Chunk::CHUNK_SIZE : 
                z / Chunk::CHUNK_SIZE
    };

    auto it = m_chunks.find(chunkPos);
    if (it == m_chunks.end()) {
        return BlockType::AIR;
    }

    int localX = x - (chunkPos.x * Chunk::CHUNK_SIZE);
    int localZ = z - (chunkPos.z * Chunk::CHUNK_SIZE);

    return it->second->getBlock(localX, y, localZ);
}

void World::placeBlock(const glm::ivec3 &pos, BlockType type)
{
    ChunkPos chunkPos = {
        (pos.x < 0) ? (pos.x - (Chunk::CHUNK_SIZE - 1)) / Chunk::CHUNK_SIZE : pos.x / Chunk::CHUNK_SIZE,
        (pos.z < 0) ? (pos.z - (Chunk::CHUNK_SIZE - 1)) / Chunk::CHUNK_SIZE : pos.z / Chunk::CHUNK_SIZE
    };

    if (auto it = m_chunks.find(chunkPos); it != m_chunks.end()) {
        glm::ivec3 localPos = {
            pos.x - (chunkPos.x * Chunk::CHUNK_SIZE),
            pos.y,
            pos.z - (chunkPos.z * Chunk::CHUNK_SIZE)
        };

        it->second->setBlock(localPos, type);

        updateChunkMesh(chunkPos);

        if (localPos.x == 0) 
            updateChunkMesh({chunkPos.x - 1, chunkPos.z});
        if (localPos.x == 15)
            updateChunkMesh({chunkPos.x + 1, chunkPos.z});
        if (localPos.z == 0)
            updateChunkMesh({chunkPos.x, chunkPos.z - 1});
        if (localPos.z == 15)
            updateChunkMesh({chunkPos.x, chunkPos.z + 1});
    }
}

void World::deleteBlock(const glm::ivec3 &pos)
{
    ChunkPos chunkPos = {
        (pos.x < 0) ? (pos.x - (Chunk::CHUNK_SIZE - 1)) / Chunk::CHUNK_SIZE : pos.x / Chunk::CHUNK_SIZE,
        (pos.z < 0) ? (pos.z - (Chunk::CHUNK_SIZE - 1)) / Chunk::CHUNK_SIZE : pos.z / Chunk::CHUNK_SIZE
    };

    if (auto it = m_chunks.find(chunkPos); it != m_chunks.end()) {
        glm::ivec3 localPos = {
            pos.x - (chunkPos.x * Chunk::CHUNK_SIZE),
            pos.y,
            pos.z - (chunkPos.z * Chunk::CHUNK_SIZE)
        };

        it->second->setBlock(localPos, BlockType::AIR);

        updateChunkMesh(chunkPos);
    
        if (localPos.x == 0)
            updateChunkMesh({chunkPos.x - 1, chunkPos.z});
        if (localPos.x == 15) 
            updateChunkMesh({chunkPos.x + 1, chunkPos.z});
        if (localPos.z == 0)
            updateChunkMesh({chunkPos.x, chunkPos.z - 1});
        if (localPos.z == 15)
            updateChunkMesh({chunkPos.x, chunkPos.z + 1});
    }
}

bool World::raycast(
    const Ray &ray,
    f32 maxDistance,
    RaycastResult &result
)
{
    glm::vec3 pos = ray.origin;
    glm::vec3 step = glm::sign(ray.direction);
    glm::vec3 tDelta = glm::abs(1.0f / ray.direction);
    glm::vec3 tMax;
    glm::ivec3 blockPos = glm::floor(pos);

    for (i32 i = 0; i < 3; i++) {
        if (step[i] > 0) {
            tMax[i] = ((blockPos[i] + 1) - pos[i]) * tDelta[i];
        } else {
            tMax[i] = (pos[i] - blockPos[i]) * tDelta[i];
        }
    }

    Face hitFace;
    f32 dist = 0.0f;

    while (dist < maxDistance) {
        if (tMax.x < tMax.y && tMax.x < tMax.z) {
            blockPos.x += step.x;
            dist = tMax.x;
            tMax.x += tDelta.x;
            hitFace = (step.x > 0) ? Face::WEST : Face::EAST;
        } else if (tMax.y < tMax.z) {
            blockPos.y += step.y;
            dist = tMax.y;
            tMax.y += tDelta.y;
            hitFace = (step.y > 0) ? Face::BOTTOM : Face::TOP;
        } else {
            blockPos.z += step.z;
            dist = tMax.z;
            tMax.z += tDelta.z;
            hitFace = (step.z > 0) ? Face::NORTH : Face::SOUTH;
        }

        BlockType type = getBlock(blockPos);
        if (type != BlockType::AIR) {
            result.pos = blockPos;
            result.face = hitFace;

            result.normal = blockPos;
            switch (hitFace) {
            case Face::NORTH:
                result.normal.z--;
                break;
            case Face::SOUTH:
                result.normal.z++;
                break;
            case Face::EAST:
                result.normal.x++;
                break;
            case Face::WEST:
                result.normal.x--;
                break;
            case Face::TOP:
                result.normal.y++;
                break;
            case Face::BOTTOM:
                result.normal.y--;
                break;
            }

            return true;
        }
    }

    return false;
}

bool World::checkCollision(const glm::vec3 &min, const glm::vec3 &max)
{
    i32 minX = static_cast<i32>(std::floor(min.x));
    i32 minY = static_cast<i32>(std::floor(min.y));
    i32 minZ = static_cast<i32>(std::floor(min.z));
    i32 maxX = static_cast<i32>(std::floor(max.x));
    i32 maxY = static_cast<i32>(std::floor(max.y));
    i32 maxZ = static_cast<i32>(std::floor(max.z));

    minY = std::max(minY, 0);
    maxY = std::min(maxY, Chunk::CHUNK_HEIGHT - 1);

    ChunkPos currentChunk = {0, 0};
    const Chunk *chunk = nullptr;

    for (int x = minX; x <= maxX; ++x) {
        for (int z = minZ; z <= maxZ; ++z) {
            ChunkPos chunkPos = {
                (x < 0) ? (x - (Chunk::CHUNK_SIZE - 1)) / Chunk::CHUNK_SIZE :
                    x / Chunk::CHUNK_SIZE,
                (z < 0) ? (z - (Chunk::CHUNK_SIZE - 1)) / Chunk::CHUNK_SIZE :
                    z / Chunk::CHUNK_SIZE
            };

            if (chunkPos.x != currentChunk.x || chunkPos.z != currentChunk.z) {
                currentChunk = chunkPos;
                chunk = getChunk(chunkPos);

                if (!chunk) { continue; }
            }

            i32 localX = x - (chunkPos.x * Chunk::CHUNK_SIZE);
            i32 localZ = z - (chunkPos.z * Chunk::CHUNK_SIZE);

            for (i32 y = minY; y <= maxY; ++y) {
                if (
                    chunk &&
                    chunk->getBlock(localX, y, localZ) != BlockType::AIR
                ) {
                    return true;
                }
            }
        }
    }

    return false;
}

void World::loadChunks(const ChunkPos &pos)
{
    auto chunk = std::make_unique<Chunk>();
    generateChunk(*chunk, pos);

    m_chunks[pos] = std::move(chunk);
}

void World::unloadChunks(const ChunkPos &pos)
{
    if (auto it = m_meshes.find(pos); it != m_meshes.end()) {
        it->second->destroy();
        m_meshes.erase(it);
    }

    m_chunks.erase(pos);
}

bool World::isChunkLoaded(const ChunkPos &pos)
{
    return m_chunks.find(pos) != m_chunks.end();
}

void World::generateChunk(Chunk &chunk, const ChunkPos &pos)
{
    constexpr i32 SEA_LEVEL = 64;
    constexpr i32 DIRT_DEPTH = 4;
    constexpr f32 HEIGHT_SCALE = 8.0f;
    constexpr f32 MOUNTAIN_SCALE = 16.0f;
    constexpr f32 MOUNTAIN_THRESHOLD = 0.4f;

    for (u32 x = 0; x < Chunk::CHUNK_SIZE; x++) {
        for (u32 z = 0; z < Chunk::CHUNK_SIZE; z++) {
            f32 worldX = static_cast<f32>(
                pos.x * static_cast<f32>(Chunk::CHUNK_SIZE) + x
            );
            f32 worldZ = static_cast<f32>(
                pos.z * static_cast<f32>(Chunk::CHUNK_SIZE) + z
            );

            f32 baseNoise = m_noise.GetNoise(worldX, worldZ);
            f32 mountainNoise = m_mountainNoise.GetNoise(worldX, worldZ);
            mountainNoise = (mountainNoise > MOUNTAIN_THRESHOLD) ? 
                (mountainNoise - MOUNTAIN_THRESHOLD) * MOUNTAIN_SCALE : 0.0f;

            f32 detailNoise = m_detailNoise.GetNoise(worldX, worldZ) * 0.05f;

            f32 totalHeight = baseNoise + (mountainNoise * 0.3f) + detailNoise;

            i32 height = static_cast<i32>(SEA_LEVEL + totalHeight * HEIGHT_SCALE);
            height = glm::clamp(height, SEA_LEVEL - 2, Chunk::CHUNK_HEIGHT - 1);

            for (i32 y = 0; y < Chunk::CHUNK_HEIGHT; y++) {
                if (y == 0) {
                    chunk.setBlock(x, y, z, BlockType::BEDROCK);
                } else if (y > height) {
                    chunk.setBlock(x, y, z, BlockType::AIR);
                } else if (y == height) {
                    chunk.setBlock(x, y, z, BlockType::GRASS);
                } else if (y > height - DIRT_DEPTH) {
                    chunk.setBlock(x, y, z, BlockType::DIRT);
                } else {
                    chunk.setBlock(x, y, z, BlockType::STONE);
                }
            }
        }
    }
}

const Chunk *World::getChunk(const ChunkPos &pos) const
{
    if (auto it = m_chunks.find(pos); it != m_chunks.end()) {
        return it->second.get();
    }

    return nullptr;
}

void World::updateChunkMesh(const ChunkPos &pos)
{
    if (
        std::abs(pos.x - m_playerChunkPos.x) > RENDER_DISTANCE ||
        std::abs(pos.z - m_playerChunkPos.z) > RENDER_DISTANCE
    ) {
        return;
    }

    if (auto it = m_chunks.find(pos); it != m_chunks.end()) {
        if (auto mesh = m_meshes.find(pos); mesh != m_meshes.end()) {
            std::array<const Chunk *, 4> neighbors = {
                getChunk({pos.x - 1, pos.z}),
                getChunk({pos.x + 1, pos.z}),
                getChunk({pos.x, pos.z - 1}),
                getChunk({pos.x, pos.z + 1})
            };

            mesh->second->update(*it->second, neighbors);
        }
    }
}

} // namespace wld