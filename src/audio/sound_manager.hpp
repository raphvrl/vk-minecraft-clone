#pragma once

#include <nlohmann/json.hpp>

#include <random>
#include <filesystem>

#include "audio_manager.hpp"
#include "world/blocks/block.hpp"

namespace fs = std::filesystem;

using json = nlohmann::json;

namespace sfx
{

struct Sound
{
    std::string name;
    std::vector<std::string> sounds;
    f32 volume = 1.0f;
    f32 pitch = 1.0f;
};

class SoundManager
{

public:
    static SoundManager &get()
    {
        static SoundManager instance;
        return instance;
    }

    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;

    void init();
    void destroy();
    void update();

    void playButtonClick();

    void playFootstep(const wld::BlockType &blockType, glm::vec3 &pos);
    void playPlaceBlock(const wld::BlockType &blockType, glm::vec3 &pos);
    void playBreakBlock(const wld::BlockType &blockType, glm::vec3 &pos);

    void setListenerPos(
        const glm::vec3 &pos,
        const glm::vec3 &front,
        const glm::vec3 &up
    ) {
        m_audioManager.setListenerPos(pos, front, up);
    }

    void load();
    void save();

    json toJson();
    void fromJson(const json &data);
    
    void play(const Sound &sound, bool isLooping = false);
    void play(const Sound &sound, glm::vec3 &pos, bool isLooping = false);

    void play(const Sound &sound, f32 pitch, bool isLooping = false);
    void play(const Sound &sound, glm::vec3 &pos, f32 pitch, bool isLooping = false);

    void play(const std::string &name, f32 pitch, bool isLooping = false);

    void stop(const Sound &sound);
    void stopAll();

    Sound *getSound(const std::string &name)
    {
        auto it = m_sounds.find(name);
        if (it != m_sounds.end()) {
            return &it->second;
        }
        return nullptr;
    }

    std::unordered_map<std::string, Sound> &getSounds()
    {
        return m_sounds;
    }

    void addSound(const std::string &name, const Sound &sound)
    {
        m_sounds[name] = sound;
    }

    void updateSound(const std::string &name)
    {
        auto it = m_sounds.find(name);
        if (it != m_sounds.end()) {
            auto &sounds = it->second.sounds;
            for (const auto &sound : sounds) {
                m_audioManager.loadSound(sound);
            }
        }
    }
    
    void removeSound(const std::string &name)
    {
        m_sounds.erase(name);
    }

    void renameSound(const std::string &oldName, const std::string &newName)
    {
        auto it = m_sounds.find(oldName);
        if (it != m_sounds.end()) {
            Sound sound = it->second;
            m_sounds.erase(it);
            sound.name = newName;
            m_sounds[newName] = sound;
        }
    }

private:
    SoundManager() = default;
    ~SoundManager() = default;

    AudioManager m_audioManager;

private:

    std::unordered_map<std::string, Sound> m_sounds;

    std::mt19937 m_rng{std::random_device{}()};

    void processSound(const std::string &id, const json &info);
};
     
} // namespace sfx