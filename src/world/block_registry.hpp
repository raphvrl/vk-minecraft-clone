#pragma once

#include <toml++/toml.hpp>

#include <map>
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
        return m_blocks.at(type);
    }

private:
    std::unordered_map<BlockType, Block> m_blocks;

};

} // namespace wld