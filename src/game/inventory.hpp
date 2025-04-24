#pragma once

#include <nlohmann/json.hpp>

#include <vector>
#include <string>
#include <fstream>
#include <filesystem>

#include "world/blocks/block_registry.hpp"

using json = nlohmann::json;

namespace fs = std::filesystem;

namespace game
{

struct ItemSlot
{
    wld::BlockType type = wld::BlockType::AIR;
    int quantity = 0;

    bool isEmpty() const { return quantity == 0 && type == wld::BlockType::AIR; }
    void clear() { type = wld::BlockType::AIR; quantity = 0; }
};

class Inventory
{

public:
    Inventory();
    ~Inventory() = default;

    ItemSlot &getSlot(u32 index);

    void setSelectedSlot(u32 index);
    u32 getSelectedSlot() const { return m_selectedSlot; }

    ItemSlot &getSelectedItemSlot() { return m_slots[m_selectedSlot]; }

    void save();
    void load();

    json toJson() const;
    void fromJson(const json &data);

private:
    static constexpr u32 HOTBAR_SIZE = 9;

    std::vector<ItemSlot> m_slots;
    u32 m_selectedSlot = 0;

    static constexpr char *CONFIG_PATH = "saves/inventory.json";

    void initDefaultInventory();

};

} // namespace game