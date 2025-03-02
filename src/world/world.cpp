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

    m_pipelines[P_OPAQUE] = gfx::Pipeline::Builder(*m_ctx)
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

    m_pipelines[P_TRANSPARENT] = gfx::Pipeline::Builder(*m_ctx)
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
        .setCullMode(VK_CULL_MODE_NONE)
        .setBlending(true)
        .build();

    m_texture.init(*m_ctx, "terrain.png");
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

    for (usize i = 0; i < m_pipelines.size(); i++) {
        m_descriptorSets[i] = m_pipelines[i].createDescriptorSet(descriptors);
    }

    m_chunks.reserve(RENDER_DISTANCE * RENDER_DISTANCE);
    m_meshes.reserve(RENDER_DISTANCE * RENDER_DISTANCE);

    m_generator.init(0);

    update({0.0f, 0.0f, 0.0f});
}

void World::destroy()
{
    m_ubo.destroy();
    m_texture.destroy();
    
    for (auto &pipeline : m_pipelines) {
        pipeline.destroy();
    }

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
    
    const f32 squaredDist = RENDER_DISTANCE * RENDER_DISTANCE;

    if (newPos != m_playerChunkPos) {
        m_chunksNeeded.clear();
        m_chunksToLoad.clear();
        m_chunksToUnload.clear();

        std::queue<ChunkPos> empty;
        std::swap(m_pendingChunks, empty);
        std::swap(m_pendingMeshes, empty);

        for (int x = -RENDER_DISTANCE; x <= RENDER_DISTANCE; x++) {
            for (int z = -RENDER_DISTANCE; z <= RENDER_DISTANCE; z++) {
                if (x * x + z * z > squaredDist) continue;

                ChunkPos pos = {newPos.x + x, newPos.z + z};

                m_chunksNeeded.insert(pos);

                if (!isChunkLoaded(pos)) {
                    f32 maxDist = static_cast<f32>(x * x + z * z);
                    m_chunksToLoad.push_back({pos, maxDist});
                }
            }
        }

        std::sort(
            m_chunksToLoad.begin(),
            m_chunksToLoad.end(),
            [](const auto &a, const auto &b) {
                return a.second < b.second;
            }
        );

        for (const auto &[pos, dist] : m_chunksToLoad) {
            m_pendingChunks.push(pos);
        }

        for (const auto &[pos, chunk] : m_chunks) {
            if (m_chunksNeeded.find(pos) == m_chunksNeeded.end()) {
                m_chunksToUnload.push_back(pos);
            }
        }

        for (const auto &pos : m_chunksToUnload) {
            unloadChunks(pos);
        }

        m_playerChunkPos = newPos;
    }

    int chunksLoaded = 0;
    while (!m_pendingChunks.empty() && chunksLoaded < CHUNKS_PER_TICK) {
        ChunkPos pos = m_pendingChunks.front();
        m_pendingChunks.pop();

        if (!isChunkLoaded(pos) && m_chunksNeeded.find(pos) != m_chunksNeeded.end()) {
            loadChunks(pos);
            
            m_pendingMeshes.push(pos);
            
            ChunkPos neighbors[4] = {
                {pos.x - 1, pos.z},
                {pos.x + 1, pos.z},
                {pos.x, pos.z - 1},
                {pos.x, pos.z + 1}
            };
            
            for (const auto& neighborPos : neighbors) {
                if (
                    isChunkLoaded(neighborPos)
                    && m_chunksNeeded.find(neighborPos) != m_chunksNeeded.end()
                ) {
                    m_pendingMeshes.push(neighborPos);
                }
            }
            
            chunksLoaded++;
        }
    }

    int meshesUpdated = 0;
    while (!m_pendingMeshes.empty() && meshesUpdated < CHUNKS_PER_TICK) {
        ChunkPos pos = m_pendingMeshes.front();
        m_pendingMeshes.pop();

        if (isChunkLoaded(pos)) {
            updateMeshe(pos);
            meshesUpdated++;
        }
    }

    m_playerChunkPos = newPos;
}

void World::render(const core::Camera &camera)
{
    m_frustum = core::Frustum::fromViewProj(
        camera.getView(),
        camera.getProj()
    );

    m_pipelines[P_OPAQUE].bind();
    m_pipelines[P_OPAQUE].bindDescriptorSet(m_descriptorSets[P_OPAQUE]);

    UniformBufferObject uniformBuffer = {
        .view = camera.getView(),
        .proj = camera.getProj(),
        .camPos = camera.getPos()
    };

    m_ubo.update(&uniformBuffer, sizeof(UniformBufferObject));

    for (const auto &[pos, mesh] : m_meshes) {
        f32 x = static_cast<f32>(pos.x * Chunk::CHUNK_SIZE);
        f32 z = static_cast<f32>(pos.z * Chunk::CHUNK_SIZE);

        glm::vec3 min(x, 0.0f, z);
        glm::vec3 max(
            x + Chunk::CHUNK_SIZE,
            Chunk::CHUNK_HEIGHT,
            z + Chunk::CHUNK_SIZE
        );

        if (!m_frustum.isBoxVisible(min, max)) {
            continue;
        }

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, {x, 0.0f, z});

        m_pipelines[P_OPAQUE].push(
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(model),
            &model
        );

        mesh->drawOpaque();
    }

    m_pipelines[P_TRANSPARENT].bind();
    m_pipelines[P_TRANSPARENT].bindDescriptorSet(
        m_descriptorSets[P_TRANSPARENT]
    );

    for (const auto &[pos, mesh] : m_meshes) {
        f32 x = static_cast<f32>(pos.x * Chunk::CHUNK_SIZE);
        f32 z = static_cast<f32>(pos.z * Chunk::CHUNK_SIZE);

        glm::vec3 min(x, 0.0f, z);
        glm::vec3 max(
            x + Chunk::CHUNK_SIZE,
            Chunk::CHUNK_HEIGHT,
            z + Chunk::CHUNK_SIZE
        );

        if (!m_frustum.isBoxVisible(min, max)) {
            continue;
        }

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, {x, 0.0f, z});

        m_pipelines[P_TRANSPARENT].push(
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(model),
            &model
        );

        mesh->drawTransparent();
    }
}

BlockType World::getBlock(int x, int y, int z) const
{
    if (y < 0 || y >= Chunk::CHUNK_HEIGHT) {
        return BlockType::AIR;
    }

    ChunkPos chunkPos(
        (x < 0) ?(x - (Chunk::CHUNK_SIZE - 1)) / Chunk::CHUNK_SIZE :
            x / Chunk::CHUNK_SIZE,
        (z < 0) ? (z - (Chunk::CHUNK_SIZE - 1)) / Chunk::CHUNK_SIZE :
            z / Chunk::CHUNK_SIZE
    );

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

        m_pendingMeshes.push(chunkPos);

        if (localPos.x == 0)
            m_pendingMeshes.push({chunkPos.x - 1, chunkPos.z});
        if (localPos.x == Chunk::CHUNK_SIZE - 1)
            m_pendingMeshes.push({chunkPos.x + 1, chunkPos.z});
        if (localPos.z == 0)
            m_pendingMeshes.push({chunkPos.x, chunkPos.z - 1});
        if (localPos.z == Chunk::CHUNK_SIZE - 1)
            m_pendingMeshes.push({chunkPos.x, chunkPos.z + 1});
    }
}

void World::deleteBlock(const glm::ivec3 &pos)
{
    placeBlock(pos, BlockType::AIR);
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
        if (
            type != BlockType::AIR &&
            m_blockRegistry.getBlock(type).collision
        ) {
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

    ChunkPos currentChunk = {INT_MAX, INT_MAX};
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
                BlockType block = chunk->getBlock(localX, y, localZ);

                if (
                    chunk &&
                    block != BlockType::AIR &&
                    m_blockRegistry.getBlock(block).collision
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
    m_generator.generateChunk(*chunk, pos);

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

const Chunk *World::getChunk(const ChunkPos &pos) const
{
    if (auto it = m_chunks.find(pos); it != m_chunks.end()) {
        return it->second.get();
    }

    return nullptr;
}

void World::updateMeshe(const ChunkPos &pos)
{
    auto chunk = getChunk(pos);

    if (!chunk) { return; }

    std::array<const Chunk *, 4> neighbors = {
        getChunk({pos.x - 1, pos.z}),
        getChunk({pos.x + 1, pos.z}),
        getChunk({pos.x, pos.z - 1}),
        getChunk({pos.x, pos.z + 1})
    };

    if (auto it = m_meshes.find(pos); it != m_meshes.end()) {
        it->second->update(*chunk, neighbors);
    } else {
        auto mesh = std::make_unique<ChunkMesh>();
        mesh->init(*m_ctx, m_blockRegistry);
        mesh->generate(*chunk, neighbors);
        m_meshes[pos] = std::move(mesh);
    }
}

} // namespace wld