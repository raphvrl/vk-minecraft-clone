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
        u32 data;

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
            attributeDescriptions[2].offset = offsetof(Vertex, data);

            return attributeDescriptions;
        }
    };

    struct MeshData
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
    };

    ChunkMesh() = default;
    virtual ~ChunkMesh() = default;

    ChunkMesh(const ChunkMesh &) = delete;
    ChunkMesh &operator=(const ChunkMesh &) = delete;

    void init(gfx::VulkanCtx &ctx);
    void destroy();

    void generate(MeshData &meshData);

    void draw();

    static MeshData calculateMeshData(
        const Chunk &chunk,
        std::array<const Chunk *, 4> &neighbors,
        const BlockRegistry &registryconst 
    );

private:
    // vulkan
    gfx::VulkanCtx *m_ctx;
    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;

    VkBuffer m_vertexBuffer;
    VmaAllocation m_vertexAllocation;
    VkBuffer m_indexBuffer;
    VmaAllocation m_indexAllocation;

    void createVertexBuffer();
    void createIndexBuffer();

    // mesh generation
    static const std::array<glm::vec3, 4> FACE_NORTH;
    static const std::array<glm::vec3, 4> FACE_SOUTH;
    static const std::array<glm::vec3, 4> FACE_EAST;
    static const std::array<glm::vec3, 4> FACE_WEST;
    static const std::array<glm::vec3, 4> FACE_TOP;
    static const std::array<glm::vec3, 4> FACE_BOTTOM;

    static void addFace(
        const glm::vec3 &pos,
        const std::array<glm::vec3, 4> &vertices,
        const std::array<glm::vec2, 4> &uvs,
        BlockType block,
        std::vector<Vertex> &verticesData,
        std::vector<u32> &indicesData
    );

    static std::array<glm::vec2, 4> getUVs(
        BlockType block,
        Face face,
        const BlockRegistry &registry
    );

    static bool isFaceVisible(
        const Chunk &chunk,
        std::array<const Chunk *, 4> neighbors,
        i32 x,
        i32 y,
        i32 z,
        BlockType block,
        const BlockRegistry &registry
    );
};

} // namespace wld