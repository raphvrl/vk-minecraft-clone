#include "sound_cache.hpp"

namespace sfx
{

void SoundCache::load(const fs::path &path)
{
    std::string name = path.stem().string();

    if (m_buffers.find(name) != m_buffers.end()) {
        throw std::runtime_error("Sound already loaded: " + name);
    }

    SoundBuffer buffer;
    buffer.load(path);
    m_buffers[path.string()] = buffer;
}

void SoundCache::remove(const fs::path &path)
{
    std::string name = path.stem().string();

    auto it = m_buffers.find(name);
    if (it != m_buffers.end()) {
        it->second.destroy();
        m_buffers.erase(it);
    } else {
        throw std::runtime_error("Sound not found: " + name);
    }
}

void SoundCache::reload(const fs::path &path)
{
    std::string name = path.stem().string();

    auto it = m_buffers.find(name);
    if (it != m_buffers.end()) {
        remove(path);
        load(path);
    } else {
        throw std::runtime_error("Sound not found: " + name);
    }
}

void SoundCache::destroy()
{
    for (auto &pair : m_buffers) {
        pair.second.destroy();
    }
    m_buffers.clear();
}

ALuint SoundCache::getSoundID(const std::string &name) const
{
    auto it = m_buffers.find(name);
    if (it == m_buffers.end()) {
        throw std::runtime_error("Sound not found: " + name);
    }
    return it->second.getId();
}

} // namespace sfx