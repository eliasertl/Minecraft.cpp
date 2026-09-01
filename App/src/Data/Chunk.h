#pragma once

#include "Sizes.h"

#include <glm/glm.hpp>

#define MINECRAFT_CHUNK_USE_BOUNDS_CHECK

namespace Minecraft::Data
{
    typedef byte BlockID;
    typedef byte BlockMeta;
    constexpr uint16_t CHUNK_LENGTH = 16;
    constexpr uint16_t CHUNK_WIDTH = 16;
    constexpr uint16_t CHUNK_HEIGHT = 256;

    struct World;

    struct Block
    {
        BlockID id;
        BlockMeta meta;
    };

    class Chunk
    {
    public:
        Chunk();
        Chunk(const Chunk& other);
        Chunk& operator=(const Chunk& other);
        ~Chunk();

        Block *getBlock(uint16_t x, uint16_t y, uint16_t z);
        void setBlock(uint16_t x, uint16_t y, uint16_t z, BlockID id, BlockMeta meta = 0);
        glm::ivec2 getChunkPosition() const { return m_Position; }
        void markDirty() { m_IsDirty = true; }
        void markClean() { m_IsDirty = false; }
        bool isDirty() const { return m_IsDirty; }

        operator bool() const { return m_Blocks != nullptr; }
        operator Block*() { return m_Blocks; }

        World *getWorld() const { return m_World; }

    private:
        // position of the chunk in chunks, not blocks
        glm::ivec2 m_Position = {0, 0};
        World *m_World = nullptr;

        Block *m_Blocks;
        bool m_IsDirty;

        friend class World;
    };
}