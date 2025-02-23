#include "block_registry.hpp"

namespace wld
{

void BlockRegistry::load(const std::string &path)
{
    auto config = toml::parse_file(path);

    auto blocks = config["blocks"];
    for (auto &&[blockKey, blockData] : *blocks.as_table()) {
        int id;
        Block block;
        TextureInfo texInfo;

        if (auto blockTable = blockData.as_table()) {
            if (auto textures = blockTable->at("textures").as_table()) {
                for (auto &&[face, coords] : *textures) {
                    auto pos = coords.as_table();
                    int x = pos->at("x").value_or(-1);
                    int y = pos->at("y").value_or(-1);

                    if (face == "all") {
                        texInfo.fill({x, y});
                    } else if (face == "sides") {
                        texInfo.sides({x, y});
                    } else if (face == "north") {
                        texInfo.set(Face::NORTH, {x, y});
                    } else if (face == "south") {
                        texInfo.set(Face::SOUTH, {x, y});
                    } else if (face == "east") {
                        texInfo.set(Face::EAST, {x, y});
                    } else if (face == "west") {
                        texInfo.set(Face::WEST, {x, y});
                    } else if (face == "top") {
                        texInfo.set(Face::TOP, {x, y});
                    } else if (face == "bottom") {
                        texInfo.set(Face::BOTTOM, {x, y});
                    }
                }
            }

            id = blockTable->at("id").value_or(-1);
            block.setName(std::string(blockKey.str()));
            block.setTextureInfo(texInfo);

            m_blocks[static_cast<BlockType>(id)] = block;
        }
    }
}

} // namespace wld