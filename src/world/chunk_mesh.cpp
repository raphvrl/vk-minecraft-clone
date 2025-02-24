#include "chunk_mesh.hpp"
#include "block_registry.hpp"
#include "world.hpp"

namespace wld
{

void ChunkMesh::init(gfx::VulkanCtx &ctx, const BlockRegistry &blockRegistry)
{
    m_ctx = &ctx;
    m_blockRegistry = &blockRegistry;
}

void ChunkMesh::destroy()
{
    vkDeviceWaitIdle(m_ctx->getDevice());

    vmaDestroyBuffer(
        m_ctx->getAllocator(),
        m_vertexBuffer,
        m_vertexAllocation
    );

    vmaDestroyBuffer(
        m_ctx->getAllocator(),
        m_indexBuffer,
        m_indexAllocation
    );
}

void ChunkMesh::generateMesh(
    const Chunk &chunk,
    std::array<const Chunk *, 4> &neighbors
)
{
    m_vertices.clear();
    m_indices.clear();

    for (u32 y = 0; y < Chunk::CHUNK_HEIGHT; y++) {
        for (u32 z = 0; z < Chunk::CHUNK_SIZE; z++) {
            for (u32 x = 0; x < Chunk::CHUNK_SIZE; x++) {
                BlockType block = chunk.getBlock(x, y, z);
                if (block == BlockType::AIR) {
                    continue;
                }

                glm::vec3 pos(x, y, z);

                if (isFaceVisible(chunk, neighbors, x - 1, y, z)) {
                    addFace(pos, FACE_WEST, getUVs(block, Face::WEST), block);
                }

                if (isFaceVisible(chunk, neighbors, x + 1, y, z)) {
                    addFace(pos, FACE_EAST, getUVs(block, Face::EAST), block);
                }

                if (isFaceVisible(chunk, neighbors, x, y - 1, z)) {
                    addFace(pos, FACE_BOTTOM, getUVs(block, Face::BOTTOM), block);
                }

                if (isFaceVisible(chunk, neighbors, x, y + 1, z)) {
                    addFace(pos, FACE_TOP, getUVs(block, Face::TOP), block);
                }

                if (isFaceVisible(chunk, neighbors, x, y, z + 1)) {
                    addFace(pos, FACE_NORTH, getUVs(block, Face::NORTH), block);
                }

                if (isFaceVisible(chunk, neighbors, x, y, z - 1)) {
                    addFace(pos, FACE_SOUTH, getUVs(block, Face::SOUTH), block);
                }
            }
        }
    }

    createVertexBuffer();
    createIndexBuffer();
}

void ChunkMesh::draw()
{
    VkCommandBuffer commandBuffer = m_ctx->getCommandBuffer();

    VkDeviceSize offsets[] = {0};

    vkCmdBindVertexBuffers(
        commandBuffer,
        0,
        1,
        &m_vertexBuffer,
        offsets
    );

    vkCmdBindIndexBuffer(
        commandBuffer,
        m_indexBuffer,
        0,
        VK_INDEX_TYPE_UINT32
    );

    vkCmdDrawIndexed(
        commandBuffer,
        static_cast<u32>(m_indices.size()),
        1,
        0,
        0,
        0
    );
}

void ChunkMesh::createVertexBuffer()
{
    VkDeviceSize bufferSize = sizeof(m_vertices[0]) * m_vertices.size();

    VkBuffer stagingBuffer;
    VmaAllocation stagingAllocation;

    m_ctx->createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_MEMORY_USAGE_CPU_ONLY,
        stagingBuffer,
        stagingAllocation
    );

    void *data;
    vmaMapMemory(m_ctx->getAllocator(), stagingAllocation, &data);
    memcpy(data, m_vertices.data(), bufferSize);
    vmaUnmapMemory(m_ctx->getAllocator(), stagingAllocation);

    m_ctx->createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY,
        m_vertexBuffer,
        m_vertexAllocation
    );

    m_ctx->copyBuffer(stagingBuffer, m_vertexBuffer, bufferSize);

    vmaDestroyBuffer(
        m_ctx->getAllocator(),
        stagingBuffer,
        stagingAllocation
    );
}

void ChunkMesh::createIndexBuffer()
{
    VkDeviceSize bufferSize = sizeof(m_indices[0]) * m_indices.size();

    VkBuffer stagingBuffer;
    VmaAllocation stagingAllocation;

    m_ctx->createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_MEMORY_USAGE_CPU_ONLY,
        stagingBuffer,
        stagingAllocation
    );

    void *data;
    vmaMapMemory(m_ctx->getAllocator(), stagingAllocation, &data);
    memcpy(data, m_indices.data(), bufferSize);
    vmaUnmapMemory(m_ctx->getAllocator(), stagingAllocation);

    m_ctx->createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY,
        m_indexBuffer,
        m_indexAllocation
    );

    m_ctx->copyBuffer(stagingBuffer, m_indexBuffer, bufferSize);

    vmaDestroyBuffer(
        m_ctx->getAllocator(),
        stagingBuffer,
        stagingAllocation
    );
}

const std::array<glm::vec3, 4> ChunkMesh::FACE_NORTH = {
    glm::vec3(1.0f, 0.0f, 1.0f),
    glm::vec3(0.0f, 0.0f, 1.0f),
    glm::vec3(0.0f, 1.0f, 1.0f),
    glm::vec3(1.0f, 1.0f, 1.0f)
};

const std::array<glm::vec3, 4> ChunkMesh::FACE_SOUTH = {
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(1.0f, 1.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f)
};

const std::array<glm::vec3, 4> ChunkMesh::FACE_EAST = {
    glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(1.0f, 0.0f, 1.0f),
    glm::vec3(1.0f, 1.0f, 1.0f),
    glm::vec3(1.0f, 1.0f, 0.0f)
};

const std::array<glm::vec3, 4> ChunkMesh::FACE_WEST = {
    glm::vec3(0.0f, 0.0f, 1.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 1.0f)
};

const std::array<glm::vec3, 4> ChunkMesh::FACE_TOP = {
    glm::vec3(1.0f, 1.0f, 1.0f),
    glm::vec3(0.0f, 1.0f, 1.0f),
    glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(1.0f, 1.0f, 0.0f)
};

const std::array<glm::vec3, 4> ChunkMesh::FACE_BOTTOM = {
    glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 1.0f),
    glm::vec3(1.0f, 0.0f, 1.0f)
};

void ChunkMesh::addFace(
    const glm::vec3 &pos,
    const std::array<glm::vec3, 4> &vertices,
    const std::array<glm::vec2, 4> &uvs,
    BlockType block
)
{
    UNUSED(block);

    u32 indexOffset = m_vertices.size();

    for (usize i = 0; i < 4; i++) {
        Vertex vertex;
        vertex.pos = pos + vertices[i];
        vertex.uv = uvs[i];
        m_vertices.push_back(vertex);
    }

    m_indices.push_back(indexOffset + 0);
    m_indices.push_back(indexOffset + 1);
    m_indices.push_back(indexOffset + 2);
    m_indices.push_back(indexOffset + 2);
    m_indices.push_back(indexOffset + 3);
    m_indices.push_back(indexOffset + 0);
}

std::array<glm::vec2, 4> ChunkMesh::getUVs(BlockType block, Face face)
{
    f32 tileSize = 16.0f / 256.0f;

    TextureInfo texInfo = m_blockRegistry->getBlock(block).getTextureInfo();
    glm::uvec2 uv = texInfo.getUV(face);

    f32 x = (uv.x * tileSize);
    f32 y = (1.0f - (uv.y + 1) * tileSize);

    return {
        glm::vec2(x, y),
        glm::vec2(x + tileSize, y),
        glm::vec2(x + tileSize, y + tileSize),
        glm::vec2(x, y + tileSize)
    };
}

bool ChunkMesh::isFaceVisible(
    const Chunk &chunk,
    std::array<const Chunk *, 4> neighbors,
    i32 x,
    i32 y,
    i32 z
)
{
    if (x < 0) {
        if (neighbors[0] == nullptr) {
            return true;
        }

        return neighbors[0]->getBlock(
            Chunk::CHUNK_SIZE - 1, y, z
        ) == BlockType::AIR;
    }

    if (x >= Chunk::CHUNK_SIZE) {
        if (neighbors[1] == nullptr) {
            return true;
        }

        return neighbors[1]->getBlock(
            0, y, z
        ) == BlockType::AIR;
    }

    if (z < 0) {
        if (neighbors[2] == nullptr) {
            return true;
        }

        return neighbors[2]->getBlock(
            x, y, Chunk::CHUNK_SIZE - 1
        ) == BlockType::AIR;
    }

    if (z >= Chunk::CHUNK_SIZE) {
        if (neighbors[3] == nullptr) {
            return true;
        }

        return neighbors[3]->getBlock(
            x, y, 0
        ) == BlockType::AIR;
    }

    return chunk.getBlock(x, y, z) == BlockType::AIR;
}

} // namespace wld