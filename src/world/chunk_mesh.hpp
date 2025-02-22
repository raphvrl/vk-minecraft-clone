#pragma once

namespace wld
{

class ChunkMesh
{

public:
    ChunkMesh() = default;
    virtual ~ChunkMesh() = default;

    ChunkMesh(const ChunkMesh &) = delete;
    ChunkMesh &operator=(const ChunkMesh &) = delete;

private:
    

};

} // namespace wld