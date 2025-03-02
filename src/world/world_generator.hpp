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

    static constexpr i32 SEA_LEVEL = 64;
    static constexpr i32 WORD_HEIGHT = 128;
    static constexpr i32 BASE_TERRAIN_HEIGHT = 64;
    static constexpr f32 TERRAIN_SCALE = 0.015f;
    static constexpr f32 TERRAIN_HEIGHT_SCALE = 12.0f;

    FastNoiseLite m_terrainNoise;
    FastNoiseLite m_detailNoise;

    BlockType getBlockAtHeight(i32 y, i32 terrainHeight, i32 dirtDepth);
    i32 getTerrainHeight(i32 x, i32 z);
};

} // namespace wld