#pragma once

#include <glm/ext.hpp>
#include <toml++/toml.hpp>

#include "graphics/vulkan_ctx.hpp"

namespace wld
{

// forward declarations
class BlockRegistry;
class World;
class Chunk;
struct ChunkPos;

enum class BlockType;

enum class Face
{
    NORTH,
    SOUTH,
    EAST,
    WEST,
    TOP,
    BOTTOM
};

class ChunkMesh
{

public:
    struct Vertex
    {
        glm::vec3 pos;
        glm::vec2 uv;
        u32 lightLevel;

        static VkVertexInputBindingDescription getBindingDescription()
        {
            VkVertexInputBindingDescription bindingDescription = {};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Vertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        }

        static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
        {
            std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex, pos);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex, uv);

            attributeDescriptions[2].binding = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = VK_FORMAT_R32_UINT;
            attributeDescriptions[2].offset = offsetof(Vertex, lightLevel);

            return attributeDescriptions;
        }
    };

    ChunkMesh() = default;
    virtual ~ChunkMesh() = default;

    ChunkMesh(const ChunkMesh &) = delete;
    ChunkMesh &operator=(const ChunkMesh &) = delete;

    void init(gfx::VulkanCtx &ctx, BlockRegistry &registry);
    void destroy();

    void generate(
        const Chunk &chunk,
        const std::array<const Chunk *, 4> &neighbors
    );

    void update(
        const Chunk &chunk,
        const std::array<const Chunk *, 4> &neighbors
    );

    void drawOpaque();
    void drawTransparent();

private:
    gfx::VulkanCtx *m_ctx;
    BlockRegistry *m_registry;

    std::vector<Vertex> m_vertices;
    std::vector<u32> m_indices;

    VkBuffer m_vertexBuffer;
    VmaAllocation m_vertexAllocation;
    VkBuffer m_indexBuffer;
    VmaAllocation m_indexAllocation;

    std::vector<Vertex> m_transparentVertices;
    std::vector<u32> m_transparentIndices;

    VkBuffer m_vertexBufferTransparent;
    VmaAllocation m_vertexAllocationTransparent;
    VkBuffer m_indexBufferTransparent;
    VmaAllocation m_indexAllocationTransparent;

    void createVertexBuffer(
        VkBuffer &buffer,
        VmaAllocation &allocation,
        const std::vector<Vertex> &vertices
    );

    void createIndexBuffer(
        VkBuffer &buffer,
        VmaAllocation &allocation,
        const std::vector<u32> &indices
    );

    // mesh generation
    static const std::array<glm::vec3, 4> FACE_NORTH;
    static const std::array<glm::vec3, 4> FACE_SOUTH;
    static const std::array<glm::vec3, 4> FACE_EAST;
    static const std::array<glm::vec3, 4> FACE_WEST;
    static const std::array<glm::vec3, 4> FACE_TOP;
    static const std::array<glm::vec3, 4> FACE_BOTTOM;

    void addFace(
        const Chunk &chunk,
        const std::array<const Chunk *, 4> &neighbors,
        const glm::vec3 &pos,
        const std::array<glm::vec3, 4> &vertices,
        const std::array<glm::vec2, 4> &uvs,
        BlockType block
    );

    std::array<glm::vec2, 4> getUVs(
        BlockType block,
        Face face
    );

    bool isFaceVisible(
        const Chunk &chunk,
        std::array<const Chunk *, 4> neighbors,
        int x,
        int y,
        int z,
        BlockType block
    );

    glm::vec3 getNormalFromFace(std::array<glm::vec3, 4> &face);
    u8 getFaceLightLevel(
        const Chunk &chunk,
        const std::array<const Chunk *, 4> &neighbors,
        int x,
        int y,
        int z
    );
};

} // namespace wld