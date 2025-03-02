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

    int operationsThisFrame = 0;

    checkPendingChunks(operationsThisFrame);
    checkPendingMeshes(operationsThisFrame);

    if (operationsThisFrame >= OPPERATIONS_PER_FRAME) {
        return;
    }

    if (newPos.x == m_playerChunkPos.x && newPos.z == m_playerChunkPos.z) {
        return;
    }

    m_chunksNeeded.clear();
    m_chunksToLoad.clear();
    m_chunksToUnload.clear();

    const f32 maxDistSquared = RENDER_DISTANCE * RENDER_DISTANCE;

    for (int x = -RENDER_DISTANCE; x <= RENDER_DISTANCE; ++x) {
        for (int z = -RENDER_DISTANCE; z <= RENDER_DISTANCE; ++z) {
            if (x * x + z * z > maxDistSquared) continue;

            ChunkPos pos = {newPos.x + x, newPos.z + z};
            m_chunksNeeded.insert(pos);

            if (!isChunkLoaded(pos) && !isChunkPending(pos)) {
                f32 maxDistSquared = static_cast<f32>(x * x + z * z);
                m_chunksToLoad.push_back({pos, maxDistSquared});
            }
        }
    }

    std::sort(m_chunksToLoad.begin(), m_chunksToLoad.end(),
        [](const auto &a, const auto &b) {
            return a.second < b.second;
        }
    );

    {
        std::lock_guard<std::mutex> lock(m_chunkMutex);
        for (const auto &[pos, _] : m_chunks) {
            if (m_chunksNeeded.find(pos) == m_chunksNeeded.end()) {
                m_chunksToUnload.push_back(pos);
            }
        }
    }

    for (const auto &pos : m_chunksToUnload) {
        unloadChunks(pos);
    }

    usize chunkLimit = std::min(
        m_chunksToLoad.size(),
        static_cast<usize>(CHUNKS_PER_FRAME * 2)
    );

    for (usize i = 0; i < chunkLimit; i++) {
        const auto &pos = m_chunksToLoad[i].first;

        if (isChunkPending(pos)) continue;

        ChunkTask task;
        task.pos = pos;
        task.future = m_threadPool.enqueue(
            [this, pos]() {
                auto chunk = std::make_unique<Chunk>();
                m_generator.generateChunk(*chunk, pos);
                return chunk;
            }
        );

        m_chunkTasks.push_back(std::move(task));
    }

    if (m_chunksToLoad.size() <= CHUNKS_PER_FRAME) {
        m_playerChunkPos = newPos;
    }
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

        queueMeshGeneration(chunkPos);

        if (localPos.x == 0) 
            queueMeshGeneration({chunkPos.x - 1, chunkPos.z});
        if (localPos.x == 15)
            queueMeshGeneration({chunkPos.x + 1, chunkPos.z});
        if (localPos.z == 0)
            queueMeshGeneration({chunkPos.x, chunkPos.z - 1});
        if (localPos.z == 15)
            queueMeshGeneration({chunkPos.x, chunkPos.z + 1});
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

bool World::isChunkPending(const ChunkPos &pos)
{
    for (const auto &chunk : m_chunkTasks) {
        if (chunk.pos == pos) {
            return true;
        }
    }

    return false;
}

void World::checkPendingChunks(int &operationsThisFrame)
{
    std::vector<ChunkPos> completedChunks;
    int maxToProcess = OPPERATIONS_PER_FRAME - operationsThisFrame;
    int processed = 0;

    auto it = m_chunkTasks.begin();
    while (it != m_chunkTasks.end() && processed < maxToProcess) {
        if (it->future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            auto pos = it->pos;
            {
                std::lock_guard<std::mutex> lock(m_chunkMutex);
                m_chunks[pos] = it->future.get();
            }
            completedChunks.push_back(pos);
            it = m_chunkTasks.erase(it);
            processed++;
        } else {
            ++it;
        }
    }

    operationsThisFrame += processed;

    int chunkLimit = std::min(
        static_cast<int>(completedChunks.size()),
        OPPERATIONS_PER_FRAME - operationsThisFrame
    );

    for (int i = 0; i < chunkLimit; i++) {
        queueMeshGeneration(completedChunks[i]);
        operationsThisFrame++;
    } 

    for (const auto &pos : completedChunks) {
        ChunkPos neighbors[4] = {
            {pos.x - 1, pos.z},
            {pos.x + 1, pos.z},
            {pos.x, pos.z - 1},
            {pos.x, pos.z + 1}
        };
        
        for (const auto &neighborPos : neighbors) {
            if (isChunkLoaded(neighborPos) && !isMeshPending(neighborPos)) {
                queueMeshGeneration(neighborPos);
            }
        }
        
        queueMeshGeneration(pos);
    }
}

bool World::isMeshPending(const ChunkPos &pos)
{
    return std::any_of(
        m_meshTasks.begin(),
        m_meshTasks.end(),
        [&pos](const MeshTask &task) { return task.pos == pos; });
}

void World::checkPendingMeshes(int &operationsThisFrame)
{
    int maxToProcess = OPPERATIONS_PER_FRAME - operationsThisFrame;
    int processed = 0;

    auto it = m_meshTasks.begin();
    while (it != m_meshTasks.end() && processed < maxToProcess) {
        if (it->future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
            auto pos = it->pos;
            auto meshData = it->future.get();
            
            if (auto chunkIt = m_chunks.find(pos); chunkIt != m_chunks.end()) {
                if (auto meshIt = m_meshes.find(pos); meshIt != m_meshes.end()) {
                    meshIt->second->destroy();
                    m_meshes.erase(meshIt);
                }
                
                auto newMesh = std::make_unique<ChunkMesh>();
                newMesh->init(*m_ctx);
                newMesh->generate(meshData);
                m_meshes[pos] = std::move(newMesh);
            }
            
            processed++;
            it = m_meshTasks.erase(it);
        } else {
            ++it;
        }
    }

    operationsThisFrame += processed;
}

void World::queueMeshGeneration(const ChunkPos &pos)
{
    if (isMeshPending(pos)) return;

    MeshTask task;
    task.pos = pos;
    task.future = m_threadPool.enqueue([this, pos]() {
        std::lock_guard<std::mutex> lock(m_chunkMutex);
        
        auto chunkIt = m_chunks.find(pos);
        if (chunkIt == m_chunks.end()) {
            return ChunkMesh::MeshData{};
        }
        
        std::array<const Chunk*, 4> neighbors = {
            getChunk({pos.x - 1, pos.z}),
            getChunk({pos.x + 1, pos.z}),
            getChunk({pos.x, pos.z - 1}),
            getChunk({pos.x, pos.z + 1})
        };

        return ChunkMesh::calculateMeshData(
            *chunkIt->second,
            neighbors, m_blockRegistry
        );
    });
    
    m_meshTasks.push_back(std::move(task));
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
    std::lock_guard<std::mutex> lock(m_chunkMutex);
    return m_chunks.find(pos) != m_chunks.end();
}

const Chunk *World::getChunk(const ChunkPos &pos) const
{
    if (auto it = m_chunks.find(pos); it != m_chunks.end()) {
        return it->second.get();
    }

    return nullptr;
}

} // namespace wld