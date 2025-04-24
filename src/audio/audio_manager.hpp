#pragma once

#include <AL/al.h>
#include <AL/alc.h>

#include <glm/glm.hpp>

#include <string>
#include <vector>

#include "sound_cache.hpp"

namespace sfx
{

class AudioManager
{

public:

    AudioManager() = default;
    ~AudioManager() = default;
    
    void init();
    void destroy();

    void update();

    void play(
        const std::string &name,
        f32 volume = 1.0f,
        f32 pitch = 1.0f,
        bool isLooping = false
    );

    void play(
        const std::string &name,
        glm::vec3 pos,
        f32 volume = 1.0f,
        f32 pitch = 1.0f, 
        bool isLooping = false
    );

    void stopAll();
    void stop(const std::string &name);

    void setListenerPos(
        const glm::vec3 &pos,
        const glm::vec3& front,
        const glm::vec3& up
    );

public:
    void loadSound(const fs::path &path) {
        m_soundCache.load(path);
    }

    void reloadSound(const fs::path &path) {
        m_soundCache.reload(path);
    }

private:
    ALCdevice *m_device = nullptr;
    ALCcontext *m_context = nullptr;

    SoundCache m_soundCache;

private:
    struct ActiveSound {
        ALuint sourceId;
        std::string name;
        bool isLooping;
    };

    std::vector<ActiveSound> m_activeSounds;

};

} // namespace sfx