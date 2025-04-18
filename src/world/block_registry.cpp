#include "block_registry.hpp"

namespace wld
{

BlockRegistry::BlockRegistry()
{
    std::ifstream file("assets/config/blocks.json");
    nlohmann::json config = nlohmann::json::parse(file);

    auto atlas = config["atlas"];
    int count = atlas.value("count", 0);
    m_blocks.resize(count);

    auto blocks = config["blocks"];

    for (auto it = blocks.begin(); it != blocks.end(); ++it) {
        Block block;
        TextureInfo texInfo;
        int id = -1;

        std::string blockKey = it.key();
        auto blockData = it.value();

        if (blockData.is_object()) {
            if (blockData.contains("textures") && blockData["textures"].is_object()) {
                auto textures = blockData["textures"];
                for (auto texIt = textures.begin(); texIt != textures.end(); ++texIt) {
                    std::string face = texIt.key();
                    auto coords = texIt.value();
                    
                    int x = coords.value("x", -1);
                    int y = coords.value("y", -1);

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

            id = blockData.value("id", -1);
            block.name = blockKey;
            block.textures = texInfo;

            if (blockData.contains("transparency")) {
                block.transparency = blockData["transparency"];
            }

            if (blockData.contains("collision")) {
                block.collision = blockData["collision"];
            }

            if (blockData.contains("breakable")) {
                block.breakable = blockData["breakable"];
            }

            if (blockData.contains("cross")) {
                block.cross = blockData["cross"];
            }

            if (blockData.contains("material")) {
                block.material = blockData["material"];
            }
        }

        m_blocks[id] = block;
    }
}

} // namespace wld