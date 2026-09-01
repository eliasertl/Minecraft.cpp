#include "Chunk.h"
#include <cstdlib>
#include <cstring>

#include <tracy/Tracy.hpp>

namespace Minecraft::Data
{
    Chunk::Chunk()
    {
        ZoneScoped;
        m_Blocks = static_cast<Block*>(malloc(sizeof(Block) * CHUNK_LENGTH * CHUNK_WIDTH * CHUNK_HEIGHT));
        memset(m_Blocks, 0, sizeof(Block) * CHUNK_LENGTH * CHUNK_WIDTH * CHUNK_HEIGHT);
    }

    Chunk::Chunk(const Chunk &other)
    {
        ZoneScoped;
        m_Position = other.m_Position;
        m_IsDirty = other.m_IsDirty;
        m_World = other.m_World;

        m_Blocks = static_cast<Block*>(malloc(sizeof(Block) * CHUNK_LENGTH * CHUNK_WIDTH * CHUNK_HEIGHT));
        memcpy(m_Blocks, other.m_Blocks, sizeof(Block) * CHUNK_LENGTH * CHUNK_WIDTH * CHUNK_HEIGHT);
    }

    Chunk& Chunk::operator=(const Chunk &other)
    {
        ZoneScoped;
        if (this != &other)
        {
            m_Position = other.m_Position;
            m_IsDirty = other.m_IsDirty;
            m_World = other.m_World;

            if (m_Blocks)
                free(m_Blocks);

            m_Blocks = static_cast<Block*>(malloc(sizeof(Block) * CHUNK_LENGTH * CHUNK_WIDTH * CHUNK_HEIGHT));
            memcpy(m_Blocks, other.m_Blocks, sizeof(Block) * CHUNK_LENGTH * CHUNK_WIDTH * CHUNK_HEIGHT);
        }
        return *this;
    }

    Chunk::~Chunk()
    {
        ZoneScoped;
        free(m_Blocks);
    }

    Block* Chunk::getBlock(uint16_t x, uint16_t y, uint16_t z)
    {
        ZoneScoped;
        #ifdef MINECRAFT_CHUNK_USE_BOUNDS_CHECK
        if (x >= CHUNK_LENGTH || y >= CHUNK_HEIGHT || z >= CHUNK_WIDTH)
            return nullptr;
        #endif

        return &m_Blocks[x + y * CHUNK_LENGTH + z * CHUNK_LENGTH * CHUNK_HEIGHT];
    }

    void Chunk::setBlock(uint16_t x, uint16_t y, uint16_t z, BlockID id, BlockMeta meta)
    {
        #ifdef MINECRAFT_CHUNK_USE_BOUNDS_CHECK
        if (x >= CHUNK_LENGTH || y >= CHUNK_HEIGHT || z >= CHUNK_WIDTH)
            return;
        #endif

        Block* block = &m_Blocks[x + y * CHUNK_LENGTH + z * CHUNK_LENGTH * CHUNK_HEIGHT];
        block->id = id;
        block->meta = meta;
        markDirty();
    }
}