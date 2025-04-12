#pragma once

#include <FastNoiseLite.h>
#include <random>

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
    void generateTree(Chunk &chunk, int x, int y, int z, std::mt19937 &rng);
    bool canPlaceTree(Chunk &chunk, int x, int y, int z);

    void generateFlowers(Chunk &chunk, int x, int y, int z, std::mt19937 &rng);

private:
    u32 m_seed;
    std::mt19937 m_rng;

    FastNoiseLite m_terrainNoise;
    FastNoiseLite m_biomeNoise;
    FastNoiseLite m_treeNoise;
    FastNoiseLite m_flowerNoise;

};

} // namespace wld