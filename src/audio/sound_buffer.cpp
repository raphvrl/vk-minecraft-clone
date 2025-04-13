#include "sound_buffer.hpp"
#include <stb/stb_vorbis.h>

namespace sfx
{

void SoundBuffer::load(const fs::path &path)
{
    if (!fs::exists(path)) {
        throw std::runtime_error("File does not exist: " + path.string());
    }

    std::string extension = path.extension().string();
    if (extension != ".ogg") {
        throw std::runtime_error("Unsupported file format: " + extension);
    }

    alGenBuffers(1, &m_id);
    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        throw std::runtime_error(
            "Failed to generate OpenAL buffer : " + std::to_string(error)
        );
    }

    loadOggFile(path);
}

void SoundBuffer::destroy()
{
    if (m_id) {
        alDeleteBuffers(1, &m_id);
        ALenum error = alGetError();
        if (error != AL_NO_ERROR) {
            throw std::runtime_error(
                "Failed to delete OpenAL buffer: " + std::to_string(error)
            );
        }
    }
}

void SoundBuffer::loadOggFile(const fs::path &path)
{
    int channels, sampleRate;
    short *data = nullptr;

    int numSamples = stb_vorbis_decode_filename(
        path.string().c_str(), &channels, &sampleRate, &data
    );

    if (numSamples < 0) {
        throw std::runtime_error("Failed to decode Ogg file: " + path.string());
    }

    ALenum format = (channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

    ALsizei size = numSamples * channels * sizeof(short);
    alBufferData(m_id, format, data, size, sampleRate);
    ALenum error = alGetError();

    if (error != AL_NO_ERROR) {
        free(data);
        throw std::runtime_error(
            "Failed to buffer OpenAL data: " + std::to_string(error)
        );
    }
}

} // namespace sfx