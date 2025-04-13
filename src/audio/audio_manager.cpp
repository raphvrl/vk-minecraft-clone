#include "audio_manager.hpp"

namespace sfx
{

void AudioManager::init()
{
    m_device = alcOpenDevice(nullptr);
    if (!m_device) {
        throw std::runtime_error("Failed to open audio device");
    }

    m_context = alcCreateContext(m_device, nullptr);
    if (!m_context) {
        alcCloseDevice(m_device);
        throw std::runtime_error("Failed to create audio context");
    }

    alcMakeContextCurrent(m_context);
}

void AudioManager::destroy()
{
    stopAll();

    for (const auto &sound : m_activeSounds) {
        alSourcei(sound.sourceId, AL_BUFFER, 0);
        alDeleteSources(1, &sound.sourceId);
    }

    m_activeSounds.clear();

    m_soundCache.destroy();

    alcMakeContextCurrent(nullptr);
    alcDestroyContext(m_context);
    alcCloseDevice(m_device);
}

void AudioManager::update()
{
    auto it = m_activeSounds.begin();
    while (it != m_activeSounds.end()) {
        ALint state;
        alGetSourcei(it->sourceId, AL_SOURCE_STATE, &state);
        if (state == AL_STOPPED) {
            alDeleteSources(1, &it->sourceId);
            it = m_activeSounds.erase(it);
        } else {
            ++it;
        }
    }
}

void AudioManager::play(const std::string &name, bool isLooping)
{
    ALuint bufferId = m_soundCache.getSoundID(name);
    
    ALuint sourceId = 0;
    alGenSources(1, &sourceId);

    alSourcei(sourceId, AL_BUFFER, bufferId);
    alSourcei(sourceId, AL_LOOPING, isLooping ? AL_TRUE : AL_FALSE);

    alSourcei(sourceId, AL_SOURCE_RELATIVE, AL_TRUE);
    alSource3f(sourceId, AL_POSITION, 0, 0, 0);

    alSourcef(sourceId, AL_GAIN, 1.0f);

    alSourcePlay(sourceId);

    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        alDeleteSources(1, &sourceId);
        throw std::runtime_error("Failed to play sound: " + name);
    }

    m_activeSounds.push_back({sourceId, name, isLooping});
}

void AudioManager::play(const std::string &name, glm::vec3 pos, bool isLooping)
{
    ALuint bufferId = m_soundCache.getSoundID(name);
    
    ALuint sourceId = 0;
    alGenSources(1, &sourceId);

    alSourcei(sourceId, AL_BUFFER, bufferId);
    alSourcei(sourceId, AL_LOOPING, isLooping ? AL_TRUE : AL_FALSE);

    alSource3f(sourceId, AL_POSITION, pos.x, pos.y, pos.z);

    alSourcef(sourceId, AL_REFERENCE_DISTANCE, 1.0f);
    alSourcef(sourceId, AL_MAX_DISTANCE, 16.0f);
    alSourcef(sourceId, AL_ROLLOFF_FACTOR, 1.0f);

    alSourcePlay(sourceId);

    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        alDeleteSources(1, &sourceId);
        throw std::runtime_error("Failed to play sound: " + name);
    }

    m_activeSounds.push_back({sourceId, name, isLooping});
}

void AudioManager::stopAll()
{
    for (const auto &sound : m_activeSounds) {
        alSourceStop(sound.sourceId);
    }

    alGetError();
}

void AudioManager::stop(const std::string &name)
{
    auto it = m_activeSounds.begin();
    while (it != m_activeSounds.end()) {
        if (it->name == name) {
            alSourceStop(it->sourceId);
            alDeleteSources(1, &it->sourceId);
            it = m_activeSounds.erase(it);
        } else {
            ++it;
        }
    }
}

void AudioManager::setListenerPos(
    const glm::vec3 &pos,
    const glm::vec3& front,
    const glm::vec3& up
)
{
    alListener3f(AL_POSITION, pos.x, pos.y, pos.z);
    alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    ALfloat orientation[] = {
        front.x, front.y, front.z,
        up.x, up.y, up.z
    };
    alListenerfv(AL_ORIENTATION, orientation);
}


} // namespace sfx