#pragma once

#include "Sizes.h"

#define MINECRAFT_CHUNK_USE_BOUNDS_CHECK

namespace Minecraft::Data
{
    typedef byte BlockID;
    typedef byte BlockMeta;
    constexpr uint16_t CHUNK_LENGTH = 16;

    struct Block
    {
        BlockID id;
        BlockMeta meta;
    };

    class Chunk
    {
    public:
        Chunk();
        ~Chunk();

        Block* getBlock(uint16_t x, uint16_t y, uint16_t z);
        void setBlock(uint16_t x, uint16_t y, uint16_t z, BlockID id, BlockMeta meta);
        void markDirty() { m_IsDirty = true; }
        void markClean() { m_IsDirty = false; }

    private:
        Block* m_Blocks;
        bool m_IsDirty;
    };
}