#include "inventory.hpp"

namespace game
{

Inventory::Inventory()
{
    m_slots.resize(HOTBAR_SIZE);

    fs::path configPath = CONFIG_PATH;
    if (!fs::exists(configPath)) {
        fs::create_directories(configPath.parent_path());
    }
}

ItemSlot &Inventory::getSlot(u32 index)
{
    if (index < m_slots.size()) {
        return m_slots[index];
    } else {
        throw std::out_of_range("Index out of range");
    } 
}

void Inventory::setSelectedSlot(u32 index)
{
    if (index < m_slots.size()) {
        m_selectedSlot = index;
    } else {
        throw std::out_of_range("Index out of range");
    }
}

void Inventory::save()
{
    json data = toJson();

    std::ofstream file(CONFIG_PATH);
    if (file.is_open()) {
        file << data.dump(4);
        file.close();
    } else {
        throw std::runtime_error("Unable to open file for saving inventory");
    }
}

void Inventory::load()
{
    std::ifstream file(CONFIG_PATH);
    if (file.is_open()) {
        json data;
        file >> data;
        fromJson(data);
        file.close();
    } else {
        throw std::runtime_error("Unable to open file for loading inventory");
    }
}

json Inventory::toJson() const
{
    json data;
    
    data["selected"] = m_selectedSlot;
    
    json slotsJson = json::array();
    for (const auto &slot : m_slots) {
        json slotJson;
        slotJson["type"] = static_cast<int>(slot.type);
        slotJson["quantity"] = slot.quantity;
        slotsJson.push_back(slotJson);
    }

    data["slots"] = slotsJson;
    return data;
}

void Inventory::fromJson(const json &data)
{
    m_selectedSlot = data["selected"].get<u32>();
    
    const auto &slotsJson = data["slots"];
    for (size_t i = 0; i < slotsJson.size(); ++i) {
        if (i < m_slots.size()) {
            m_slots[i].type = static_cast<wld::BlockType>(slotsJson[i]["type"].get<int>());
            m_slots[i].quantity = slotsJson[i]["quantity"].get<int>();
        }
    }
}

void Inventory::initDefaultInventory()
{
    for (auto& slot : m_slots) {
        slot.clear();
    }

    m_selectedSlot = 0;

    ItemSlot &slot = getSlot(0);
    slot.type = wld::BlockType::GRASS;
    slot.quantity = 64;

    slot = getSlot(1);
    slot.type = wld::BlockType::DIRT;
    slot.quantity = 64;
}

} // namespace game