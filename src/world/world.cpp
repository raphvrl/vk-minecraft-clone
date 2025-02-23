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
        .setShader(gfx::ShaderType::VERTEX, "default.vert.spv")
        .setShader(gfx::ShaderType::FRAGMENT, "default.frag.spv")
        .addPushConstant(gfx::ShaderType::VERTEX, 0, sizeof(glm::mat4))
        .setVertexInput(
            &binding,
            attributes.data(),
            attributes.size()
        )
        .addDescriptorBinding({
            .binding = 0,
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .count = 1,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT
        })
        .build();

    m_texture.init(*m_ctx, "blocks.png");
    m_blockSet = m_pipeline.createDescriptorSet(m_texture);

    update({0.0f, 0.0f, 0.0f});
}

void World::destroy()
{
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
    std::vector<ChunkPos> chunksToLoad;
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
                    chunksToLoad.push_back(pos);
                }
            }
        }
    }

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

    for (usize i = 0; i < chunkToLoadThisFrame; i++) {
        loadChunks(chunksToLoad[i]);
    }

    std::vector<ChunkPos> meshesToUpdate;

    for (usize i = 0; i < chunkToLoadThisFrame; i++) {
        const auto &pos = chunksToLoad[i];
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
            mesh->generateMesh(*chunk->second, *this, {pos.x, pos.z});
            m_meshes[pos] = std::move(mesh);
        }
    }

    if (chunksToLoad.size() <= CHUNKS_PER_FRAME) {
        m_playerChunkPos = newPos;
    }
}

void World::render(const core::Camera &camera)
{
    glm::mat4 view = camera.getView();
    glm::mat4 proj = camera.getProj();

    m_pipeline.bind();

    m_pipeline.bindDescriptorSet(m_blockSet);

    for (const auto &[pos, mesh] : m_meshes) {
        f32 x = static_cast<f32>(pos.x * Chunk::CHUNK_SIZE);
        f32 z = static_cast<f32>(pos.z * Chunk::CHUNK_SIZE);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, {x, 0.0f, z});

        glm::mat4 mvp = proj * view * model;
        m_pipeline.push(gfx::ShaderType::VERTEX, 0, sizeof(glm::mat4), &mvp);

        mesh->draw();
    }
}

BlockType World::getBlock(int x, int y, int z) const
{
    if (y < 0 || y >= Chunk::CHUNK_HEIGHT) {
        return BlockType::AIR;
    }

    ChunkPos chunkPos;
    if (x < 0) {
        chunkPos.x = (x - (Chunk::CHUNK_SIZE - 1)) / Chunk::CHUNK_SIZE;
    } else {
        chunkPos.x = x / Chunk::CHUNK_SIZE;
    }
    
    if (z < 0) {
        chunkPos.z = (z - (Chunk::CHUNK_SIZE - 1)) / Chunk::CHUNK_SIZE;
    } else {
        chunkPos.z = z / Chunk::CHUNK_SIZE;
    }

    int localX = x - (chunkPos.x * Chunk::CHUNK_SIZE);
    int localZ = z - (chunkPos.z * Chunk::CHUNK_SIZE);

    auto it = m_chunks.find(chunkPos);
    if (it == m_chunks.end()) {
        return BlockType::AIR;
    }

    return it->second->getBlock(localX, y, localZ);
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

void World::generateChunk(Chunk &chunk, const ChunkPos &pos) {
    UNUSED(pos);

    for (u32 y = 0; y < Chunk::CHUNK_HEIGHT; y++) {
        for (u32 z = 0; z < Chunk::CHUNK_SIZE; z++) {
            for (u32 x = 0; x < Chunk::CHUNK_SIZE; x++) {
                if (y == 0) {
                    chunk.setBlock(x, y, z, BlockType::STONE);
                } else if (y == 1) {
                    chunk.setBlock(x, y, z, BlockType::DIRT);
                } else if (y == 2) {
                    chunk.setBlock(x, y, z, BlockType::GRASS);
                } else {
                    chunk.setBlock(x, y, z, BlockType::AIR);
                }
            }
        }
    }
}

} // namespace wld