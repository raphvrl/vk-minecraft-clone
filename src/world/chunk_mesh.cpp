#include "chunk_mesh.hpp"
#include "block_registry.hpp"
#include "world.hpp"

namespace wld
{

void ChunkMesh::init(gfx::VulkanCtx &ctx, BlockRegistry &registry)
{
    m_ctx = &ctx;
    m_registry = &registry;
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

    vmaDestroyBuffer(
        m_ctx->getAllocator(),
        m_vertexBufferTransparent,
        m_vertexAllocationTransparent
    );

    vmaDestroyBuffer(
        m_ctx->getAllocator(),
        m_indexBufferTransparent,
        m_indexAllocationTransparent
    );
}

void ChunkMesh::generate(
    const Chunk &chunk,
    const std::array<const Chunk *, 4> &neighbors
)
{
    for (u32 y = 0; y < Chunk::CHUNK_HEIGHT; y++) {
        for (u32 z = 0; z < Chunk::CHUNK_SIZE; z++) {
            for (u32 x = 0; x < Chunk::CHUNK_SIZE; x++) {
                BlockType block = chunk.getBlock(x, y, z);
                if (block == BlockType::AIR) {
                    continue;
                }

                glm::vec3 pos(x, y, z);

                if (isFaceVisible(chunk, neighbors, x - 1, y, z, block)) {
                    addFace(
                        pos,
                        ChunkMesh::FACE_WEST,
                        getUVs(block, Face::WEST),
                        block
                    );
                }

                if (isFaceVisible(chunk, neighbors, x + 1, y, z, block)) {
                    addFace(
                        pos,
                        ChunkMesh::FACE_EAST,
                        getUVs(block, Face::EAST),
                        block
                    );
                }

                if (isFaceVisible(chunk, neighbors, x, y - 1, z, block)) {
                    addFace(
                        pos,
                        ChunkMesh::FACE_BOTTOM,
                        getUVs(block, Face::BOTTOM),
                        block
                    );
                }

                if (isFaceVisible(chunk, neighbors, x, y + 1, z, block)) {
                    addFace(
                        pos,
                        ChunkMesh::FACE_TOP,
                        getUVs(block, Face::TOP),
                        block
                    );
                }

                if (isFaceVisible(chunk, neighbors, x, y, z + 1, block)) {
                    addFace(
                        pos,
                        ChunkMesh::FACE_NORTH,
                        getUVs(block, Face::NORTH),
                        block
                    );
                }

                if (isFaceVisible(chunk, neighbors, x, y, z - 1, block)) {
                    addFace(
                        pos,
                        ChunkMesh::FACE_SOUTH,
                        getUVs(block, Face::SOUTH),
                        block
                    );
                }
            }
        }
    };   

    createVertexBuffer(m_vertexBuffer, m_vertexAllocation, m_vertices);
    createIndexBuffer(m_indexBuffer, m_indexAllocation, m_indices);

    createVertexBuffer(
        m_vertexBufferTransparent,
        m_vertexAllocationTransparent,
        m_transparentVertices
    );

    createIndexBuffer(
        m_indexBufferTransparent,
        m_indexAllocationTransparent,
        m_transparentIndices
    );
}

void ChunkMesh::update(
    const Chunk &chunk,
    const std::array<const Chunk *, 4> &neighbors
)
{
    m_vertices.clear();
    m_indices.clear();
    m_transparentVertices.clear();
    m_transparentIndices.clear();

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

    vmaDestroyBuffer(
        m_ctx->getAllocator(),
        m_vertexBufferTransparent,
        m_vertexAllocationTransparent
    );

    vmaDestroyBuffer(
        m_ctx->getAllocator(),
        m_indexBufferTransparent,
        m_indexAllocationTransparent
    );

    generate(chunk, neighbors);
}

void ChunkMesh::drawOpaque()
{
    if (m_vertices.empty()) {
        return;
    }

    VkCommandBuffer cmd = m_ctx->getCommandBuffer();
    VkDeviceSize offsets[] = {0};

    vkCmdBindVertexBuffers(cmd, 0, 1, &m_vertexBuffer, offsets);
    vkCmdBindIndexBuffer(cmd, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(cmd, m_indices.size(), 1, 0, 0, 0);
}

void ChunkMesh::drawTransparent()
{
    if (m_transparentVertices.empty()) {
        return;
    }

    VkCommandBuffer cmd = m_ctx->getCommandBuffer();
    VkDeviceSize offsets[] = {0};

    vkCmdBindVertexBuffers(cmd, 0, 1, &m_vertexBufferTransparent, offsets);

    vkCmdBindIndexBuffer(
        cmd,
        m_indexBufferTransparent,
        0,
        VK_INDEX_TYPE_UINT32
    );

    vkCmdDrawIndexed(cmd, m_transparentIndices.size(), 1, 0, 0, 0);
}

void ChunkMesh::createVertexBuffer(
    VkBuffer &buffer,
    VmaAllocation &allocation,
    const std::vector<Vertex> &vertices
)
{
    if (vertices.empty()) {
        buffer = VK_NULL_HANDLE;
        allocation = VK_NULL_HANDLE;
        return;
    }

    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

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
    memcpy(data, vertices.data(), bufferSize);
    vmaUnmapMemory(m_ctx->getAllocator(), stagingAllocation);

    m_ctx->createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY,
        buffer,
        allocation
    );

    m_ctx->copyBuffer(stagingBuffer, buffer, bufferSize);

    vmaDestroyBuffer(
        m_ctx->getAllocator(),
        stagingBuffer,
        stagingAllocation
    );
}

void ChunkMesh::createIndexBuffer(
    VkBuffer &buffer,
    VmaAllocation &allocation,
    const std::vector<u32> &indices
)
{
    if (indices.empty()) {
        buffer = VK_NULL_HANDLE;
        allocation = VK_NULL_HANDLE;
        return;
    }

    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

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
    memcpy(data, indices.data(), bufferSize);
    vmaUnmapMemory(m_ctx->getAllocator(), stagingAllocation);

    m_ctx->createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY,
        buffer,
        allocation
    );

    m_ctx->copyBuffer(stagingBuffer, buffer, bufferSize);

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
    std::vector<Vertex> *verticesData;
    std::vector<u32> *indicesData;

    if (m_registry->getBlock(block).transparency) {
        verticesData = &m_transparentVertices;
        indicesData = &m_transparentIndices;
    } else {
        verticesData = &m_vertices;
        indicesData = &m_indices;
    }

    u32 indexOffset = verticesData->size();

    u32 r, g, b;
    r = 255,
    g = 255,
    b = 255;

    u32 encodedData =
        (u32(r)) |
        (u32(g) << 8) |
        (u32(b) << 16) |
        (u32(0) << 24);

    std::array<glm::vec3, 4> adjustedVerts = vertices;

    if (block == BlockType::WATER) {
        f32 heightScale = 0.875f;
        for (int i = 0; i < 4; i++) {
            adjustedVerts[i].y *= heightScale;
        }
    }

    for (usize i = 0; i < 4; i++) {
        Vertex vertex;
        vertex.pos = pos + adjustedVerts[i];
        vertex.uv = uvs[i];
        vertex.data = encodedData;
        verticesData->push_back(vertex);
    }

    indicesData->push_back(indexOffset + 0);
    indicesData->push_back(indexOffset + 1);
    indicesData->push_back(indexOffset + 2);
    indicesData->push_back(indexOffset + 2);
    indicesData->push_back(indexOffset + 3);
    indicesData->push_back(indexOffset + 0);
}

std::array<glm::vec2, 4> ChunkMesh::getUVs(
    BlockType block,
    Face face
)
{
    f32 tileSize = 16.0f / 256.0f;

    TextureInfo texInfo = m_registry->getBlock(block).textures;
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
    i32 z,
    BlockType block
)
{
    BlockType adjacentBlock;
    bool isChunkBoundary = false;
    
    if (x < 0) {
        if (neighbors[0] == nullptr) return true;
        adjacentBlock = neighbors[0]->getBlock(Chunk::CHUNK_SIZE - 1, y, z);
        isChunkBoundary = true;
    } else if (x >= Chunk::CHUNK_SIZE) {
        if (neighbors[1] == nullptr) return true;
        adjacentBlock = neighbors[1]->getBlock(0, y, z);
        isChunkBoundary = true;
    } else if (z < 0) {
        if (neighbors[2] == nullptr) return true;
        adjacentBlock = neighbors[2]->getBlock(x, y, Chunk::CHUNK_SIZE - 1);
        isChunkBoundary = true;
    } else if (z >= Chunk::CHUNK_SIZE) {
        if (neighbors[3] == nullptr) return true;
        adjacentBlock = neighbors[3]->getBlock(x, y, 0);
        isChunkBoundary = true;
    } else if (y < 0 || y >= Chunk::CHUNK_HEIGHT) {
        return true;
    } else {
        adjacentBlock = chunk.getBlock(x, y, z);
    }

    if (adjacentBlock == BlockType::AIR) {
        return true;
    }

    Block currentData = m_registry->getBlock(block);
    Block adjacentData = m_registry->getBlock(adjacentBlock);

    if (isChunkBoundary && block == adjacentBlock) {
        return false;
    }

    if (block == adjacentBlock && currentData.transparency && adjacentData.transparency) {
        return false;
    }

    if (currentData.transparency && !adjacentData.transparency) {
        return false;
    }

    if (currentData.transparency || adjacentData.transparency) {
        return true;
    }

    return false;
}

} // namespace wld