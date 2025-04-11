#pragma once

#include <FastNoiseLite.h>

#include "block.hpp"
#include "chunk.hpp"

namespace wld
{

struct ChunkPos;

class WorldGenerator
{

public:
    void init(u32 seed);
    void generateChunk(Chunk &chunk, const ChunkPos &pos);

private:
    u32 m_seed;

    FastNoiseLite m_terrainNoise;
    FastNoiseLite m_biomeNoise;

};

} // namespace wld