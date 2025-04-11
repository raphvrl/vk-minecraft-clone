#include "world_generator.hpp"
#include "chunk.hpp"

namespace wld
{

void WorldGenerator::init(u32 seed)
{
    m_seed = seed;
    m_rng.seed(seed);

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

    m_treeNoise.SetSeed(seed + 2);
    m_treeNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    m_treeNoise.SetFrequency(0.5f);
}

void WorldGenerator::generateChunk(Chunk &chunk, const ChunkPos &pos)
{
    const int seaLevel = 64;
    const int maxHeight = 128;
    const int minHeight = 1;

    int heightMap[Chunk::CHUNK_SIZE][Chunk::CHUNK_SIZE];

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
                
                if (
                    beachValue < 0.6f &&
                    height > seaLevel - 4 &&
                    height < seaLevel + 4
                ) {
                    isBeach = true;
                }
            } else if (biomeValue < 0.7f) {

                if (
                    beachValue < 0.4f &&
                    height > seaLevel - 3 &&
                    height < seaLevel + 3
                ) {
                    isBeach = true;
                }
            } else {
                height = minHeight + static_cast<int>(
                    heightValue * (maxHeight - minHeight) * 1.2f
                );
                height = std::min(height, maxHeight - 1);
            }

            heightMap[x][z] = height;

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

    std::mt19937 treeRng(m_seed + pos.x * 341873 + pos.z * 132897);

    std::vector<std::pair<int, int>> treesPlaced;

    for (int x = 0; x < Chunk::CHUNK_SIZE; ++x) {
        for (int z = 0; z < Chunk::CHUNK_SIZE; ++z) {
            int worldX = pos.x * Chunk::CHUNK_SIZE + x;
            int worldZ = pos.z * Chunk::CHUNK_SIZE + z;
            int y = heightMap[x][z];

            if (chunk.getBlock(x, y, z) == BlockType::GRASS) {
                float treeValue = m_treeNoise.GetNoise(
                    static_cast<float>(worldX * 1.2f),
                    static_cast<float>(worldZ * 1.2f)
                );
                treeValue = (treeValue + 1.0f) * 0.5f;

                float treeProbability = 0.02f;

                if (
                    m_biomeNoise.GetNoise(
                        static_cast<f32>(worldX),
                        static_cast<f32>(worldZ)
                    ) > 0.0f && 
                    m_biomeNoise.GetNoise(
                        static_cast<f32>(worldX),
                        static_cast<f32>(worldZ)
                    ) < 0.5f
                ) {
                    treeProbability = 0.10f;
                }

                bool tooCloseToExistingTree = false;
                const int minTreeDistance = 5;
                
                for (const auto& tree : treesPlaced) {
                    int dx = x - tree.first;
                    int dz = z - tree.second;
                    int distanceSquared = dx * dx + dz * dz;
                    
                    if (distanceSquared < minTreeDistance * minTreeDistance) {
                        tooCloseToExistingTree = true;
                        break;
                    }
                }
                
                if (
                    !tooCloseToExistingTree &&
                    treeValue < treeProbability &&
                    canPlaceTree(chunk, x, y, z)
                ) {
                    generateTree(chunk, x, y, z, treeRng);
                    treesPlaced.emplace_back(x, z);
                }
            }
        }
    }
}

bool WorldGenerator::canPlaceTree(Chunk &chunk, int x, int y, int z)
{
    if (x < 2 || x >= Chunk::CHUNK_SIZE - 2 || z < 2 || z >= Chunk::CHUNK_SIZE - 2) {
        return false;
    }

    for (int dy = 1; dy <= 6; ++dy) {
        if (y + dy >= Chunk::CHUNK_HEIGHT) {
            return false;
        }
        
        if (chunk.getBlock(x, y + dy, z) != BlockType::AIR) {
            return false;
        }
    }
    
    return true;
}

void WorldGenerator::generateTree(Chunk &chunk, int x, int y, int z, std::mt19937 &rng)
{
    std::uniform_int_distribution<int> trunkHeightDist(4, 6);
    int trunkHeight = trunkHeightDist(rng);

    for (int dy = 1; dy <= trunkHeight; ++dy) {
        if (y + dy < Chunk::CHUNK_HEIGHT) {
            chunk.setBlock(x, y + dy, z, BlockType::LOG);
        }
    }

    int leafStartHeight = trunkHeight - 2;
    for (int dy = leafStartHeight; dy <= trunkHeight + 1; ++dy) {
        int radius = (dy == trunkHeight + 1) ? 1 : 2;
        
        for (int dx = -radius; dx <= radius; ++dx) {
            for (int dz = -radius; dz <= radius; ++dz) {
                if (std::abs(dx) == 2 && std::abs(dz) == 2) {
                    continue;
                }
                
                int leafX = x + dx;
                int leafY = y + dy;
                int leafZ = z + dz;
                
                if (leafX >= 0 && leafX < Chunk::CHUNK_SIZE && 
                    leafY >= 0 && leafY < Chunk::CHUNK_HEIGHT && 
                    leafZ >= 0 && leafZ < Chunk::CHUNK_SIZE &&
                    chunk.getBlock(leafX, leafY, leafZ) == BlockType::AIR) {
                        
                    chunk.setBlock(leafX, leafY, leafZ, BlockType::LEAVES);
                }
            }
        }
    }
}

} // namespace wld