#include "sound_manager.hpp"
#include "world/blocks/block_registry.hpp"

namespace sfx
{


void SoundManager::init()
{
    m_audioManager.init();
    load();
}

void SoundManager::destroy()
{
    m_audioManager.destroy();
}

void SoundManager::update()
{
    m_audioManager.update();
}

void SoundManager::playButtonClick()
{
    auto it = m_sounds.find("ui.button.click");
    if (it != m_sounds.end()) {
        play(it->second, false);
    }
}

void SoundManager::playFootstep(
    const wld::BlockType &blockType, 
    glm::vec3 &pos
)
{
    auto &block = wld::BlockRegistry::get().getBlock(blockType);
    std::string &name = block.stepSound.sound;
    f32 pitch = block.stepSound.pitch;

    play(m_sounds[name], pos, pitch, false);
}

void SoundManager::playPlaceBlock(
    const wld::BlockType &blockType, 
    glm::vec3 &pos
)
{
    auto &block = wld::BlockRegistry::get().getBlock(blockType);
    std::string &name = block.placeSound.sound;
    f32 pitch = block.placeSound.pitch;

    play(m_sounds[name], pos, pitch, false);
}

void SoundManager::playBreakBlock(
    const wld::BlockType &blockType, 
    glm::vec3 &pos
)
{
    auto &block = wld::BlockRegistry::get().getBlock(blockType);
    std::string &name = block.breakSound.sound;
    f32 pitch = block.breakSound.pitch;

    play(m_sounds[name], pos, pitch, false);
}

void SoundManager::load()
{
    std::ifstream file("assets/config/sounds.json");
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open sound config file.");
    }

    json data;
    file >> data;
    fromJson(data);

    play(m_sounds["ui.button.click"], false);
}

void SoundManager::save()
{
    std::ofstream file("assets/config/sounds.json");
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open sound config file for writing.");
    }

    json data = toJson();
    file << data.dump(4);
}

void SoundManager::play(const Sound &sound, bool isLooping)
{
    if (sound.sounds.size() > 1) {
        std::uniform_int_distribution<size_t> dist(0, sound.sounds.size() - 1);
        size_t index = dist(m_rng);
        m_audioManager.play(
            sound.sounds[index],
            sound.volume,
            sound.pitch,
            isLooping
        );
    } else if (sound.sounds.size() == 1) {
        m_audioManager.play(
            sound.sounds[0],
            sound.volume,
            sound.pitch,
            isLooping
        );
    }
}

void SoundManager::play(const Sound &sound, glm::vec3 &pos, bool isLooping)
{
    if (sound.sounds.size() > 1) {
        std::uniform_int_distribution<size_t> dist(0, sound.sounds.size() - 1);
        size_t index = dist(m_rng);
        m_audioManager.play(
            sound.sounds[index],
            pos,
            sound.volume,
            sound.pitch,
            isLooping
        );
    } else if (sound.sounds.size() == 1) {
        m_audioManager.play(
            sound.sounds[0],
            pos, 
            sound.volume,
            sound.pitch,
            isLooping
        );
    }
}

void SoundManager::play(const Sound &sound, f32 pitch, bool isLooping)
{
    if (sound.sounds.size() > 1) {
        std::uniform_int_distribution<size_t> dist(0, sound.sounds.size() - 1);
        size_t index = dist(m_rng);
        m_audioManager.play(
            sound.sounds[index],
            sound.volume,
            pitch,
            isLooping
        );
    } else if (sound.sounds.size() == 1) {
        m_audioManager.play(
            sound.sounds[0],
            sound.volume,
            pitch,
            isLooping
        );
    }
}

void SoundManager::play(const Sound &sound, glm::vec3 &pos, f32 pitch, bool isLooping)
{
    if (sound.sounds.size() > 1) {
        std::uniform_int_distribution<size_t> dist(0, sound.sounds.size() - 1);
        size_t index = dist(m_rng);
        m_audioManager.play(
            sound.sounds[index],
            pos,
            sound.volume,
            pitch,
            isLooping
        );
    } else if (sound.sounds.size() == 1) {
        m_audioManager.play(
            sound.sounds[0],
            pos,
            sound.volume,
            pitch,
            isLooping
        );
    }
}

void SoundManager::play(const std::string &name, f32 pitch, bool isLooping)
{
    auto it = m_sounds.find(name);
    if (it != m_sounds.end()) {
        play(it->second, pitch, isLooping);
    }
}

void SoundManager::stop(const Sound &sound)
{
    m_audioManager.stop(sound.name);
}

void SoundManager::stopAll()
{
    m_audioManager.stopAll();
}

json SoundManager::toJson()
{
    json root;
    json categories = json::object();

    for (const auto &[id, sound] : m_sounds) {
        std::vector<std::string> parts;
        std::string currentPart;
        std::istringstream iss(id);

        while (std::getline(iss, currentPart, '.')) {
            parts.push_back(currentPart);
        }

        if (parts.size() < 1) continue;

        json* currentNode = &categories;

        for (size_t i = 0; i < parts.size() - 1; i++) {
            const std::string& part = parts[i];

            if (!currentNode->contains(part)) {
                (*currentNode)[part] = json::object();
            }

            currentNode = &(*currentNode)[part];
        }

        const std::string& lastPart = parts.back();
        (*currentNode)[lastPart] = {
            {"volume", sound.volume},
            {"pitch", sound.pitch},
            {"files", json::array()}
        };

        for (const auto &file : sound.sounds) {
            (*currentNode)[lastPart]["files"].push_back(file);
        }
    }

    root["categories"] = categories;
    return root;
}

void SoundManager::fromJson(const json &data)
{
    if (!data.contains("categories") || !data["categories"].is_object()) {
        throw std::runtime_error("Invalid sound config format.");
    }

    std::function<void(const json&, const std::string&)> processNode = 
        [this, &processNode](const json& node, const std::string& path) {
            for (auto it = node.begin(); it != node.end(); ++it) {
                const std::string& key = it.key();
                const json& value = it.value();
                
                std::string newPath = path.empty() ? key : path + "." + key;

                if (value.contains("files")) {
                    processSound(newPath, value);
                }

                else if (value.is_object()) {
                    processNode(value, newPath);
                }
            }
        };

    processNode(data["categories"], "");
}

void SoundManager::processSound(const std::string &id, const json &info)
{
    if (!info.contains("files") || !info["files"].is_array()) {
        throw std::runtime_error("Invalid sound config format.");
    }

    Sound sound;
    sound.name = id;
    sound.volume = info.value("volume", 1.0f);
    sound.pitch = info.value("pitch", 1.0f);
    
    for (const auto &file : info["files"]) {
        sound.sounds.push_back(file.get<std::string>());
        m_audioManager.loadSound(file.get<std::string>());
    }

    m_sounds[id] = sound;
}

} // namespace sfx