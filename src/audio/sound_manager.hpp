#pragma once

#include <random>
#include <filesystem>

#include "audio_manager.hpp"
#include "world/block.hpp"

namespace fs = std::filesystem;

namespace sfx
{

struct SoundGroup
{
    std::string name;
    std::vector<std::string> sounds;
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

private:
    SoundManager() = default;
    ~SoundManager() = default;

    AudioManager m_audioManager;

private:
    void registerSounds();

    std::unordered_map<std::string, std::string> m_soundMap;
    std::unordered_map<std::string, SoundGroup> m_soundGroups;

    std::mt19937 m_rng{std::random_device{}()};

    void registerSound(
        const std::string &id,
        const std::string &path
    );

    void registerGroup(
        const std::string &id,
        const std::vector<fs::path> &paths
    );

    void play(const std::string &name, bool isLooping = false)
    {
        m_audioManager.play(m_soundMap[name], isLooping);
    }

    void play(const std::string &name, glm::vec3 &pos, bool isLooping = false)
    {
        m_audioManager.play(m_soundMap[name], pos, isLooping);
    }

    void play(const SoundGroup &group, bool isLooping = false);
    void play(const SoundGroup &group, glm::vec3 &pos, bool isLooping = false);
};
     
} // namespace sfx