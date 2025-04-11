#include "world_generator.hpp"
#include "chunk.hpp"

namespace wld
{

void WorldGenerator::init(u32 seed)
{
    m_seed = seed;

    m_terrainNoise.SetSeed(seed);
    m_terrainNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    m_terrainNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    m_terrainNoise.SetFractalOctaves(5);
    m_terrainNoise.SetFrequency(0.005f);
    m_terrainNoise.SetFractalLacunarity(2.0f);
    m_terrainNoise.SetFractalGain(0.5f);

    m_biomeNoise.SetSeed(seed + 1);
    m_biomeNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    m_biomeNoise.SetFrequency(0.002f);
}

// TODO : Implement a better organization of the world generation
void WorldGenerator::generateChunk(Chunk &chunk, const ChunkPos &pos)
{
    const int seaLevel = 64;
    const int maxHeight = 128;
    const int minHeight = 1;

    for (int x = 0; x < Chunk::CHUNK_SIZE; ++x) {
        for (int z = 0; z < Chunk::CHUNK_SIZE; ++z) {
            int worldX = pos.x * Chunk::CHUNK_SIZE + x;
            int worldZ = pos.z * Chunk::CHUNK_SIZE + z;

            f32 biomeValue = m_biomeNoise.GetNoise(
                static_cast<f32>(worldX),
                static_cast<f32>(worldZ)
            );

            biomeValue = (biomeValue + 1.0f) * 0.5f;

            f32 beachValue = m_biomeNoise.GetNoise(
                static_cast<f32>(worldX * 1.5f),
                static_cast<f32>(worldZ * 1.5f)
            );
            beachValue = (beachValue + 1.0f) * 0.5f;
            

            f32 heightValue = m_terrainNoise.GetNoise(
                static_cast<f32>(worldX),
                static_cast<f32>(worldZ)
            );

            heightValue = (heightValue + 1.0f) * 0.5f;

            int height = minHeight;
            height += static_cast<int>(heightValue * (maxHeight - minHeight));
            
            bool isBeach = false;
            
            if (biomeValue < 0.45f) {
                height = minHeight + static_cast<int>(
                    heightValue * (seaLevel + 8 - minHeight)
                );
                
                if (beachValue < 0.6f && height > seaLevel - 4 && height < seaLevel + 4) {
                    isBeach = true;
                }
            } else if (biomeValue < 0.7f) {

                if (beachValue < 0.4f && height > seaLevel - 3 && height < seaLevel + 3) {
                    isBeach = true;
                }
            } else {
                height = minHeight + static_cast<int>(
                    heightValue * (maxHeight - minHeight) * 1.2f
                );
                height = std::min(height, maxHeight - 1);
            }

            for (int y = 0; y < Chunk::CHUNK_HEIGHT; ++y) {
                BlockType block = BlockType::AIR;

                if (y == 0) {
                    block = BlockType::BEDROCK;
                } else if (y <= height) {
                    bool isUnderwater = height < seaLevel - 1;

                    if (isBeach || (height <= seaLevel + 3 && biomeValue < 0.5f)) {
                        if (y == height) {
                            block = BlockType::SAND;
                        } else if (y >= height - 4) {
                            block = BlockType::SAND;
                        } else {
                            block = BlockType::STONE;
                        }
                    } else {
                        if (y == height) {
                            if (isUnderwater) {
                                block = BlockType::DIRT;
                            } else {
                                block = BlockType::GRASS;
                            }
                        } else if (y >= height - 3) {
                            block = BlockType::DIRT;
                        } else {
                            block = BlockType::STONE;
                        }
                    }
                } else if (y < seaLevel) {
                    block = BlockType::WATER;
                }
                
                chunk.setBlock(x, y, z, block);
            }
        } 
    }
}

} // namespace wld