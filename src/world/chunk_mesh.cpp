#include "chunk_mesh.hpp"
#include "block_registry.hpp"
#include "world.hpp"

namespace wld
{

void ChunkMesh::init(gfx::Device &device, BlockRegistry &registry)
{
    m_device = &device;
    m_registry = &registry;
}

void ChunkMesh::destroy()
{
    m_device->waitIdle();

    m_indexBuffer.destroy();
    m_vertexBuffer.destroy();
    m_transparentIndexBuffer.destroy();
    m_transparentVertexBuffer.destroy();
    m_crossIndexBuffer.destroy();
    m_crossVertexBuffer.destroy();
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

                
                if (m_registry->getBlock(block).cross) {
                    addFace(
                        chunk,
                        neighbors,
                        pos,
                        ChunkMesh::FACE_CROSS_1,
                        getUVs(block, Face::NORTH),
                        block,
                        Face::NORTH
                    );

                    addFace(
                        chunk,
                        neighbors,
                        pos,
                        ChunkMesh::FACE_CROSS_2,
                        getUVs(block, Face::SOUTH),
                        block,
                        Face::SOUTH
                    );

                    continue;
                }

                if (isFaceVisible(chunk, neighbors, x - 1, y, z, block)) {
                    addFace(
                        chunk,
                        neighbors,
                        pos,
                        ChunkMesh::FACE_WEST,
                        getUVs(block, Face::WEST),
                        block,
                        Face::WEST
                    );
                }

                if (isFaceVisible(chunk, neighbors, x + 1, y, z, block)) {
                    addFace(
                        chunk,
                        neighbors,
                        pos,
                        ChunkMesh::FACE_EAST,
                        getUVs(block, Face::EAST),
                        block,
                        Face::EAST
                    );
                }

                if (isFaceVisible(chunk, neighbors, x, y - 1, z, block)) {
                    addFace(
                        chunk,
                        neighbors,
                        pos,
                        ChunkMesh::FACE_BOTTOM,
                        getUVs(block, Face::BOTTOM),
                        block,
                        Face::BOTTOM
                    );
                }

                if (isFaceVisible(chunk, neighbors, x, y + 1, z, block)) {
                    addFace(
                        chunk,
                        neighbors,
                        pos,
                        ChunkMesh::FACE_TOP,
                        getUVs(block, Face::TOP),
                        block,
                        Face::TOP
                    );
                }

                if (isFaceVisible(chunk, neighbors, x, y, z + 1, block)) {
                    addFace(
                        chunk,
                        neighbors,
                        pos,
                        ChunkMesh::FACE_NORTH,
                        getUVs(block, Face::NORTH),
                        block,
                        Face::NORTH
                    );
                }

                if (isFaceVisible(chunk, neighbors, x, y, z - 1, block)) {
                    addFace(
                        chunk,
                        neighbors,
                        pos,
                        ChunkMesh::FACE_SOUTH,
                        getUVs(block, Face::SOUTH),
                        block,
                        Face::SOUTH
                    );
                }
            }
        }
    };

    m_vertexBuffer = m_device->createBuffer(
        m_vertices.size() * sizeof(Vertex),
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU 
    );

    m_vertexBuffer.uploadData(m_vertices);

    m_indexBuffer = m_device->createBuffer(
        m_indices.size() * sizeof(u32),
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU 
    );

    m_indexBuffer.uploadData(m_indices);

    m_transparentVertexBuffer = m_device->createBuffer(
        m_transparentVertices.size() * sizeof(Vertex),
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU 
    );

    m_transparentVertexBuffer.uploadData(m_transparentVertices);

    m_transparentIndexBuffer = m_device->createBuffer(
        m_transparentIndices.size() * sizeof(u32),
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU 
    );

    m_transparentIndexBuffer.uploadData(m_transparentIndices);

    m_crossVertexBuffer = m_device->createBuffer(
        m_crossVertices.size() * sizeof(Vertex),
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU 
    );

    m_crossVertexBuffer.uploadData(m_crossVertices);

    m_crossIndexBuffer = m_device->createBuffer(
        m_crossIndices.size() * sizeof(u32),
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU 
    );

    m_crossIndexBuffer.uploadData(m_crossIndices);
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
    m_crossVertices.clear();
    m_crossIndices.clear();

    m_device->waitIdle();

    m_vertexBuffer.destroy();
    m_indexBuffer.destroy();
    m_transparentVertexBuffer.destroy();
    m_transparentIndexBuffer.destroy();
    m_crossIndexBuffer.destroy();
    m_crossVertexBuffer.destroy();

    generate(chunk, neighbors);
}

void ChunkMesh::drawOpaque(VkCommandBuffer cmd)
{
    if (m_vertices.empty()) {
        return;
    }

    VkDeviceSize offsets[] = {0};
    
    VkBuffer vertexBuffer = m_vertexBuffer.getBuffer();
    vkCmdBindVertexBuffers(
        cmd,
        0,
        1,
        &vertexBuffer,
        offsets
    );

    vkCmdBindIndexBuffer(
        cmd,
        m_indexBuffer.getBuffer(),
        0,
        VK_INDEX_TYPE_UINT32
    );

    vkCmdDrawIndexed(cmd, m_indices.size(), 1, 0, 0, 0);
}

void ChunkMesh::drawTransparent(VkCommandBuffer cmd)
{
    if (m_transparentVertices.empty()) {
        return;
    }

    VkDeviceSize offsets[] = {0};

    VkBuffer vertexBuffer = m_transparentVertexBuffer.getBuffer();
    vkCmdBindVertexBuffers(
        cmd,
        0,
        1,
        &vertexBuffer,
        offsets
    );

    vkCmdBindIndexBuffer(
        cmd,
        m_transparentIndexBuffer.getBuffer(),
        0,
        VK_INDEX_TYPE_UINT32
    );

    vkCmdDrawIndexed(cmd, m_transparentIndices.size(), 1, 0, 0, 0);
}

void ChunkMesh::drawCross(VkCommandBuffer cmd)
{
    if (m_crossVertices.empty()) {
        return;
    }

    VkDeviceSize offsets[] = {0};

    VkBuffer vertexBuffer = m_crossVertexBuffer.getBuffer();
    vkCmdBindVertexBuffers(
        cmd,
        0,
        1,
        &vertexBuffer,
        offsets
    );

    vkCmdBindIndexBuffer(
        cmd,
        m_crossIndexBuffer.getBuffer(),
        0,
        VK_INDEX_TYPE_UINT32
    );

    vkCmdDrawIndexed(cmd, m_crossIndices.size(), 1, 0, 0, 0);
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

const std::array<glm::vec3, 4> ChunkMesh::FACE_CROSS_1 = {
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(1.0f, 0.0f, 1.0f),
    glm::vec3(1.0f, 1.0f, 1.0f),
    glm::vec3(0.0f, 1.0f, 0.0f)
};

const std::array<glm::vec3, 4> ChunkMesh::FACE_CROSS_2 = {
    glm::vec3(0.0f, 0.0f, 1.0f),
    glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(1.0f, 1.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 1.0f)
};

void ChunkMesh::addFace(
    const Chunk &chunk,
    const std::array<const Chunk *, 4> &neighbors,
    const glm::vec3 &pos,
    const std::array<glm::vec3, 4> &vertices,
    const std::array<glm::vec2, 4> &uvs,
    BlockType block,
    Face face
)
{
    std::vector<Vertex> *verticesData;
    std::vector<u32> *indicesData;

    if (m_registry->getBlock(block).transparency) {
        verticesData = &m_transparentVertices;
        indicesData = &m_transparentIndices;
    } else if (m_registry->getBlock(block).cross) {
        verticesData = &m_crossVertices;
        indicesData = &m_crossIndices;
    } else {
        verticesData = &m_vertices;
        indicesData = &m_indices;
    }

    u32 indexOffset = verticesData->size();

    std::array<glm::vec3, 4> adjustedVerts = vertices;

    bool adjustWaterHeight = false;
    if (block == BlockType::WATER) {
        int blockAboveY = pos.y + 1;
        BlockType blockAbove = BlockType::AIR;
        
        if (blockAboveY < Chunk::CHUNK_HEIGHT) {
            blockAbove = chunk.getBlock(pos.x, blockAboveY, pos.z);
        }

        adjustWaterHeight = (blockAbove != BlockType::WATER);
        
        if (adjustWaterHeight) {
            f32 heightScale = 0.875f;
            for (int i = 0; i < 4; i++) {
                adjustedVerts[i].y *= heightScale;
            }
        }
    }

    u8 faceLightLevel;
    if (m_registry->getBlock(block).cross) {
        faceLightLevel = getFaceLightLevel(
            chunk,
            neighbors,
            pos.x,
            pos.y,
            pos.z
        );
    } else {
        glm::vec3 normal = getNormalFromFace(adjustedVerts);
        glm::ivec3 adjPos = glm::ivec3(pos) + glm::ivec3(normal);
        faceLightLevel = getFaceLightLevel(
            chunk,
            neighbors,
            adjPos.x,
            adjPos.y,
            adjPos.z
        );
    }

    for (usize i = 0; i < 4; i++) {
        Vertex vertex;
        vertex.pos = pos + adjustedVerts[i];
        vertex.uv = uvs[i];
        vertex.lightLevel = faceLightLevel;
        vertex.faceDirection = static_cast<u32>(face);
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
    int x,
    int y,
    int z,
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

    if (m_registry->getBlock(adjacentBlock).cross) {
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

glm::vec3 ChunkMesh::getNormalFromFace(std::array<glm::vec3, 4> &face)
{
    if (face == FACE_TOP) return glm::vec3(0.0f, 1.0f, 0.0f);
    if (face == FACE_BOTTOM) return glm::vec3(0.0f, -1.0f, 0.0f);
    if (face == FACE_NORTH) return glm::vec3(0.0f, 0.0f, 1.0f);
    if (face == FACE_SOUTH) return glm::vec3(0.0f, 0.0f, -1.0f);
    if (face == FACE_EAST) return glm::vec3(1.0f, 0.0f, 0.0f);
    if (face == FACE_WEST) return glm::vec3(-1.0f, 0.0f, 0.0f);

    if (face == FACE_CROSS_1) return glm::vec3(0.0f, 1.0f, 0.0f);
    if (face == FACE_CROSS_2) return glm::vec3(0.0f, 1.0f, 0.0f);

    return glm::vec3(0.0f, 1.0f, 0.0f);
}

u8 ChunkMesh::getFaceLightLevel(
    const Chunk &chunk,
    const std::array<const Chunk *, 4> &neighbors,
    int x,
    int y,
    int z
)
{
    if (x < 0) {
        if (neighbors[0] == nullptr) return 0;
        return neighbors[0]->getLight(Chunk::CHUNK_SIZE - 1, y, z);
    } else if (x >= Chunk::CHUNK_SIZE) {
        if (neighbors[1] == nullptr) return 0;
        return neighbors[1]->getLight(0, y, z);
    } else if (z < 0) {
        if (neighbors[2] == nullptr) return 0;
        return neighbors[2]->getLight(x, y, Chunk::CHUNK_SIZE - 1);
    } else if (z >= Chunk::CHUNK_SIZE) {
        if (neighbors[3] == nullptr) return 0;
        return neighbors[3]->getLight(x, y, 0);
    } else if (y < 0 || y >= Chunk::CHUNK_HEIGHT) {
        return 0;
    } else {
        return chunk.getLight(x, y, z);
    }

    return 0;
}

} // namespace wld