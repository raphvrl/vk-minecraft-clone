#pragma once

#include <toml++/toml.hpp>

#include <vector>
#include <iostream>

#include "chunk.hpp"
#include "block.hpp"

namespace wld
{

class BlockRegistry
{

public:
    BlockRegistry() = default;

    void load(const std::string &path);

    const Block &getBlock(BlockType type) const {
        return m_blocks[static_cast<u32>(type)];
    }

    const Block &getBlock(i32 id) const {
        return m_blocks[id];
    }

private:
    std::vector<Block> m_blocks;

};

} // namespace wld