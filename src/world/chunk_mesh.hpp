#pragma once

#include <glm/ext.hpp>
#include <toml++/toml.hpp>

#include "graphics/vulkan_ctx.hpp"
#include "chunk.hpp"

namespace wld
{

class BlockRegistry;

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

        static VkVertexInputBindingDescription getBindingDescription()
        {
            VkVertexInputBindingDescription bindingDescription = {};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Vertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        }

        static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions()
        {
            std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex, pos);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex, uv);

            return attributeDescriptions;
        }
    };

    ChunkMesh() = default;
    virtual ~ChunkMesh() = default;

    ChunkMesh(const ChunkMesh &) = delete;
    ChunkMesh &operator=(const ChunkMesh &) = delete;

    void init(gfx::VulkanCtx &ctx, const BlockRegistry &blockRegistry);
    void destroy();

    void generateMesh(const Chunk &chunk);

    void draw();

private:
    const BlockRegistry *m_blockRegistry;

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

    void addFace(
        const glm::vec3 &pos,
        const std::array<glm::vec3, 4> &vertices,
        const std::array<glm::vec2, 4> &uvs,
        BlockType block
    );

    std::array<glm::vec2, 4> getUVs(BlockType block, Face face);
};

} // namespace wld