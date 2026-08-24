#include "Chunk.h"
#include <cstdlib>
#include <cstring>

namespace Minecraft::Data
{
    Chunk::Chunk()
    {
        m_Blocks = static_cast<Block*>(malloc(sizeof(Block) * CHUNK_LENGTH * CHUNK_LENGTH * CHUNK_LENGTH));
        memset(m_Blocks, 0, sizeof(Block) * CHUNK_LENGTH * CHUNK_LENGTH * CHUNK_LENGTH);
    }

    Chunk::~Chunk()
    {
        free(m_Blocks);
    }

    Block* Chunk::getBlock(uint16_t x, uint16_t y, uint16_t z)
    {
        #ifdef MINECRAFT_CHUNK_USE_BOUNDS_CHECK
        if (x >= CHUNK_LENGTH || y >= CHUNK_LENGTH || z >= CHUNK_LENGTH)
            return nullptr;
        #endif

        return &m_Blocks[x * CHUNK_LENGTH * CHUNK_LENGTH + y * CHUNK_LENGTH + z];
    }

    void Chunk::setBlock(uint16_t x, uint16_t y, uint16_t z, BlockID id, BlockMeta meta)
    {
        #ifdef MINECRAFT_CHUNK_USE_BOUNDS_CHECK
        if (x >= CHUNK_LENGTH || y >= CHUNK_LENGTH || z >= CHUNK_LENGTH)
            return;
        #endif

        m_Blocks[x * CHUNK_LENGTH * CHUNK_LENGTH + y * CHUNK_LENGTH + z].id = id;
        m_Blocks[x * CHUNK_LENGTH * CHUNK_LENGTH + y * CHUNK_LENGTH + z].meta = meta;
    }
}