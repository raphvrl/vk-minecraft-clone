#include "block_registry.hpp"

namespace wld
{

BlockRegistry::BlockRegistry()
{
    load();
}

void BlockRegistry::save()
{
    std::ofstream file(BLOCKS_CONFIG_PATH);
    if (file.is_open()) {
        file << std::setw(4) << toJson() << std::endl;
        file.close();
    } else {
        std::cerr << "Failed to open file for writing: " << BLOCKS_CONFIG_PATH << std::endl;
    }
}

void BlockRegistry::load()
{
    try {
        std::ifstream file(BLOCKS_CONFIG_PATH);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for reading: " << BLOCKS_CONFIG_PATH << std::endl;
            return;
        }
        
        json config = json::parse(file);
        m_blocks.clear();
        fromJson(config);
        
        file.close();
    } catch (const std::exception& e) {
        std::cerr << "Error loading blocks configuration: " << e.what() << std::endl;
    }
}

json BlockRegistry::toJson()
{
    json config;
    config["atlas"]["count"] = m_blocks.size();

    json blocksJson = json::object();

    for (const auto &[id, block] : m_blocks) {
        json blockData;

        blockData["id"] = id;

        json textures = json::object();

        auto &faces = block.textures.faces;
        for (int i = 0; i < 6; i++) {
            std::string faceName;
            switch (i) {
                case 0: faceName = "north"; break;
                case 1: faceName = "south"; break;
                case 2: faceName = "east"; break;
                case 3: faceName = "west"; break;
                case 4: faceName = "top"; break;
                case 5: faceName = "bottom"; break;
            }
            textures[faceName] = {{"x", faces[i].x}, {"y", faces[i].y}};
        }

        blockData["textures"] = textures;

        blockData["transparency"] = block.transparency;
        blockData["collision"] = block.collision;
        blockData["breakable"] = block.breakable;
        blockData["cross"] = block.cross;

        blockData["sounds"] = json::object();
        blockData["sounds"]["break"] = {
            {"sound", block.breakSound.sound},
            {"pitch", block.breakSound.pitch}
        };

        blockData["sounds"]["place"] = {
            {"sound", block.placeSound.sound},
            {"pitch", block.placeSound.pitch}
        };

        blockData["sounds"]["step"] = {
            {"sound", block.stepSound.sound},
            {"pitch", block.stepSound.pitch}
        };

        blockData["emission"] = block.emission;

        blocksJson[block.name] = blockData;
    }
     
    config["blocks"] = blocksJson;
    return config;
}

void BlockRegistry::fromJson(const json &config)
{
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

            if (blockData.contains("sounds") && blockData["sounds"].is_object()) {
                auto sounds = blockData["sounds"];
                if (sounds.contains("break")) {
                    block.breakSound.sound = sounds["break"].value("sound", "");
                    block.breakSound.pitch = sounds["break"].value("pitch", 1.0f);
                }

                if (sounds.contains("place")) {
                    block.placeSound.sound = sounds["place"].value("sound", "");
                    block.placeSound.pitch = sounds["place"].value("pitch", 1.0f);
                }

                if (sounds.contains("step")) {
                    block.stepSound.sound = sounds["step"].value("sound", "");
                    block.stepSound.pitch = sounds["step"].value("pitch", 1.0f);
                }
            }

            if (blockData.contains("emission")) {
                block.emission = blockData["emission"];
            }

            if (id >= 0) {
                m_blocks[id] = block;
            } else {
                m_blocks[m_blocks.size()] = block;
            }
        }
    }
}

} // namespace wld