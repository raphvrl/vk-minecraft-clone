#pragma once

#include <nlohmann/json.hpp>

#include <string>
#include <unordered_map>
#include <memory>
#include <filesystem>

#include "gui_element.hpp"
#include "graphics/device.hpp"
#include "graphics/texture_cache.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/renderer/text_renderer.hpp"
#include "graphics/renderer/block_renderer.hpp"

#include "game/inventory.hpp"

namespace fs = std::filesystem;

using json = nlohmann::json;

namespace gui
{

class GUIManager
{

public:
    GUIManager() = default;
    ~GUIManager() = default;

    void init(gfx::Device &device, gfx::TextureCache &textureCache);
    void destroy();

    void renderGame(VkCommandBuffer cmd);
    void renderMenu(VkCommandBuffer cmd);

    void loadConfig();
    void saveConfig();

    std::shared_ptr<GUIElement> getElement(const std::string &id) const;
    void addElement(const std::shared_ptr<GUIElement> &element);
    void removeElement(const std::string &id);
    void renameElement(const std::string &oldID, const std::string &newID);
    void clearAll();

    std::unordered_map<std::string, std::shared_ptr<GUIElement>> getElements() const
    {
        return m_elements;
    }

    void setInventory(game::Inventory &inventory)
    {
        m_inventory = &inventory;
    }

private:
    gfx::Device *m_device = nullptr;
    gfx::TextureCache *m_textureCache = nullptr;

    struct PushConstant
    {
        alignas(16) glm::mat4 model;
        alignas(16) glm::vec4 uv;
        alignas(4) u32 textureID;
    };

    gfx::Pipeline m_pipeline;

    gfx::ItemRenderer m_itemRenderer;
    gfx::TextRenderer m_textRenderer;

    std::unordered_map<std::string, std::shared_ptr<GUIElement>> m_elements;

    static constexpr const char* CONFIG_PATH = "assets/config/gui.json";

    void renderElement(
        VkCommandBuffer cmd,
        const std::shared_ptr<GUIElement> &element
    );

    static constexpr int ATLAS_SIZE = 256;

    glm::vec2 calculateAnchoredPosition(
        const std::shared_ptr<GUIElement> &element
    );

    f32 m_guiScale = 4.0f;

    game::Inventory *m_inventory = nullptr;

    void renderNavBar(
        VkCommandBuffer cmd,
        const std::shared_ptr<GUIElement> &element
    );

    void setElementVisible(const std::string &id, bool visible);
    void setElementPosition(const std::string &id, const glm::vec2 &position);
    void setElementSize(const std::string &id, const glm::vec2 &size);
};

} // namespace gui