#pragma once

#include <nlohmann/json.hpp>

#include <vector>
#include <iostream>
#include <fstream>
#include <map>

#include "world/chunk/chunk.hpp"
#include "block.hpp"

using json = nlohmann::json;

namespace wld
{

class BlockRegistry
{

public:
    static BlockRegistry &get()
    {
        static BlockRegistry instance;
        return instance;
    }

    BlockRegistry(const BlockRegistry &) = delete;
    BlockRegistry &operator=(const BlockRegistry &) = delete;

    u32 addBlock(const Block &block, u32 id = 0)
    {
        u32 newId = id;

        if (newId == 0 || (m_blocks.find(newId) != m_blocks.end() && m_blocks[newId].name != ""))
        {
            newId = m_blocks.size() + 1;
            while (m_blocks.find(newId) != m_blocks.end() && m_blocks[newId].name != "") {
                newId++;
            }
        }

        m_blocks[newId] = block;
        return newId;
    }

    std::map<u32, Block> &getBlocks()
    {
        return m_blocks;
    }

    Block &getBlock(BlockType type)
    {
        return getBlock(static_cast<u32>(type));
    }

    Block &getBlock(u32 id)
    {
        auto it = m_blocks.find(id);
        if (it != m_blocks.end()) {
            return it->second;
        }

        static Block emptyBlock;
        emptyBlock.name = "unknown";

        m_blocks[id] = emptyBlock;
        
        return m_blocks[id];
    }

    void changeBlockId(u32 oldId, u32 newId)
    {
        if (m_blocks.find(oldId) == m_blocks.end()) {
            return;
        }

        if (m_blocks.find(newId) != m_blocks.end()) {
            std::swap(m_blocks[oldId], m_blocks[newId]);
        } else {
            Block block = m_blocks[oldId];
            m_blocks.erase(oldId);
            m_blocks[newId] = block;
        }
    }

    void removeBlock(u32 id)
    {
        auto it = m_blocks.find(id);
        if (it != m_blocks.end()) {
            m_blocks.erase(it);
        }
    }

    void save();
    void load();

private:
    BlockRegistry();
    ~BlockRegistry() = default;

    std::map<u32, Block> m_blocks;

    const char *BLOCKS_CONFIG_PATH = "assets/config/blocks.json";

    json toJson();
    void fromJson(const json &config);

};

} // namespace wld