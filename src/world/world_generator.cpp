#include "world_generator.hpp"
#include "world.hpp"

namespace wld
{

void WorldGenerator::init(u32 seed)
{
    m_seed = seed;

    m_terrainNoise.SetSeed(seed);
    m_terrainNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    m_terrainNoise.SetFrequency(TERRAIN_SCALE);

    m_detailNoise.SetSeed(seed);
    m_detailNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    m_detailNoise.SetFrequency(TERRAIN_SCALE * 2.0f);
}

void WorldGenerator::generateChunk(Chunk &chunk, const ChunkPos &pos)
{
    const i32 chunkX = pos.x * Chunk::CHUNK_SIZE;
    const i32 chunkZ = pos.z * Chunk::CHUNK_SIZE;

    for (i32 x = 0; x < Chunk::CHUNK_SIZE; x++) {
        for (i32 z = 0; z < Chunk::CHUNK_SIZE; z++) {
            const i32 worldX = chunkX + x;
            const i32 worldZ = chunkZ + z;

            i32 terrainHeight = getTerrainHeight(worldX, worldZ);

            i32 dirtDepth = 3 + static_cast<i32>(
                m_detailNoise.GetNoise(worldX * 0.1f, worldZ * 0.1f) * 2.0f
            );

            for (i32 y = 0; y < Chunk::CHUNK_HEIGHT; y++) {
                if (y > terrainHeight) {
                    if (y <= SEA_LEVEL) {
                        chunk.setBlock(x, y, z, BlockType::WATER);
                    } else {
                        chunk.setBlock(x, y, z, BlockType::AIR);
                    }
                } else {
                    BlockType block = getBlockAtHeight(y, terrainHeight, dirtDepth);
                    chunk.setBlock(x, y, z, block);
                }
            }
        }
    }
}

BlockType WorldGenerator::getBlockAtHeight(
    i32 y,
    i32 terrainHeight,
    i32 dirtDepth
)
{
    bool isBeachZone = 
        terrainHeight >= (SEA_LEVEL - 5) && 
        terrainHeight <= (SEA_LEVEL + 1);

    if (y == terrainHeight) {
        if (isBeachZone || y <= SEA_LEVEL) {
            return BlockType::SAND;
        }
        return BlockType::GRASS;
    } else if (y > terrainHeight - dirtDepth) {
        if (isBeachZone || y <= SEA_LEVEL - 1) {
            return BlockType::SAND;
        }
        return BlockType::DIRT;
    } else if (y > 5) {
        return BlockType::STONE;
    } else {
        return BlockType::BEDROCK;
    }
}

i32 WorldGenerator::getTerrainHeight(i32 x, i32 z)
{
    f32 baseNoise = m_terrainNoise.GetNoise(
        static_cast<f32>(x),
        static_cast<f32>(z)
    );

    f32 detailNoise = m_detailNoise.GetNoise(
        static_cast<f32>(x),
        static_cast<f32>(z)
    );

    f32 CombinedNoise = baseNoise + detailNoise;

    return BASE_TERRAIN_HEIGHT + static_cast<i32>(
        CombinedNoise * TERRAIN_HEIGHT_SCALE
    );
}

} // namespace wld