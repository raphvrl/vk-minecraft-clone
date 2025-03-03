#pragma once

#include <glm/ext.hpp>

#include <vector>
#include <memory>

#include "graphics/vulkan_ctx.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/texture.hpp"
#include "graphics/uniform_buffer.hpp"
#include "text_renderer.hpp"

namespace gui
{

struct GameStat
{
    u32 fps = 0;
    u32 updatedChunks = 0;
};

enum class Mode
{
    GAME,
    PAUSE,
};

enum class Anchor
{
    TOP_LEFT,
    TOP_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_RIGHT,
    CENTER,
};

struct Element
{
    Anchor anchor;
    glm::vec2 position;
    glm::vec2 size;
    glm::vec4 uv;
};

class GUI
{

public:
    void init(gfx::VulkanCtx &ctx);
    void destroy();

    void update();
    void updateStat(const GameStat &gameStat) { m_gameStat = gameStat; }

    void render();


private:
    gfx::VulkanCtx *m_ctx;
    Mode m_mode = Mode::GAME;

    gfx::Pipeline m_pipeline;

    struct UniformBufferObject
    {
        alignas(16) glm::mat4 ortho;
    };

    gfx::UniformBuffer m_ubo;

    struct PushConstant
    {
        alignas(16) glm::mat4 model;
        alignas(16) glm::vec4 uv;
    };

    constexpr static u32 ATLAS_SIZE = 256;

    gfx::Texture m_texture;
    VkDescriptorSet m_descriptorSet;

    TextRenderer m_text;

    std::unordered_map<std::string, Element> m_elements;

    GameStat m_gameStat;

    void draw(const Element &element);

    void initGameElements();

    void drawGameElements();
};

} // namespace gui