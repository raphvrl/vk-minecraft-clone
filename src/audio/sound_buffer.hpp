#pragma once

#include <AL/al.h>

#include <fstream>
#include <filesystem>

#include "core/types.hpp"

namespace fs = std::filesystem;

namespace sfx
{

class SoundBuffer
{

public:
    SoundBuffer() = default;
    ~SoundBuffer() = default;

    void load(const fs::path &path);
    void destroy();

public:
    ALuint getId() const { return m_id; }
    ALenum getFormat() const { return m_format; }
    ALsizei getSize() const { return m_size; }

private:
    void loadOggFile(const fs::path &path);

    ALuint m_id = 0;
    ALenum m_format = 0;
    ALsizei m_size = 0;

};

} // namespace sfx