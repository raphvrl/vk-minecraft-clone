#pragma once

#include <unordered_map>

#include "sound_buffer.hpp"

namespace sfx
{

class SoundCache
{

public:
    void load(const fs::path &path);
    void remove(const fs::path &path);

    void reload(const fs::path &path);

    void destroy();

    ALuint getSoundID(const std::string &name) const;

private:
    std::unordered_map<std::string, SoundBuffer> m_buffers;
};
    
} // namespace sfx