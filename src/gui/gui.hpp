#pragma once

#include <glm/ext.hpp>

#include <vector>
#include <memory>

#include "core/window/window.hpp"
#include "graphics/vulkan_ctx.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/texture.hpp"
#include "graphics/uniform_buffer.hpp"
#include "text_renderer.hpp"
#include "game/game_state.hpp"

#include "element.hpp"
#include "button.hpp"

namespace gui
{

struct GameStat
{
    u32 fps = 0;
    u32 updatedChunks = 0;
    game::GameState state = game::GameState::RUNNING;

};

class GUI
{

public:
    void init(gfx::VulkanCtx &ctx, core::Window &window);
    void destroy();

    void update();
    void updateStat(const GameStat &gameStat) { m_gameStat = gameStat; }

    void render();

    void draw(const Element &element);

    void drawText(
        const std::string &text,
        const glm::vec2 &pos,
        f32 size,
        TextAlign align = TextAlign::TOP_LEFT
    )
    {
        m_text.draw(text, pos, size, align);
    }

    VkExtent2D getExtent() const { return m_ctx->getSwapChainExtent(); }

private:
    gfx::VulkanCtx *m_ctx;
    core::Window *m_window;

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

    std::unordered_map<std::string, gfx::Texture> m_textures;
    std::unordered_map<std::string, VkDescriptorSet> m_descriptorSets;

    TextRenderer m_text;

    std::unordered_map<std::string, Element> m_elements;
    std::unordered_map<std::string, std::unique_ptr<Button>> m_buttons;

    GameStat m_gameStat;

    void loadTexture(const std::string &name, const std::string &path);

    void initGameElements();
    void initPauseElements();

    void drawGameElements();
    void drawPauseElements();
};

} // namespace gui