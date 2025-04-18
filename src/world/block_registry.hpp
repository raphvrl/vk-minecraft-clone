#pragma once

#include <nlohmann/json.hpp>

#include <vector>
#include <iostream>

#include "chunk.hpp"
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

    const Block &getBlock(BlockType type) const {
        return m_blocks[static_cast<u32>(type)];
    }

    const Block &getBlock(int id) const {
        return m_blocks[id];
    }

private:
    BlockRegistry();
    ~BlockRegistry() = default;

    std::vector<Block> m_blocks;

};

} // namespace wld