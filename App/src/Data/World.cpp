#include "World.h"

namespace Minecraft::Data
{
    World::World()
    {
    }

    World::~World()
    {
    }

    void World::addChunk(const Chunk& chunk, const glm::ivec2& position)
    {
        Chunk storedChunk = chunk;
        storedChunk.m_Position = position;
        m_Chunks[position] = storedChunk;
    }

    void World::addEmptyChunk(const glm::ivec2& position)
    {
        Chunk chunk = Chunk();
        chunk.m_Position = position;
        m_Chunks[position] = chunk;
    }

    bool World::hasChunk(const glm::ivec2& position) const
    {
        return m_Chunks.find(position) != m_Chunks.end();
    }

    bool World::removeChunk(const glm::ivec2& position)
    {
        auto it = m_Chunks.find(position);
        if (it != m_Chunks.end())
        {
            m_Chunks.erase(it);
            return true;
        }
        return false;
    }

    Block* World::getBlock(const glm::ivec2& chunkPosition, uint16_t x, uint16_t y, uint16_t z)
    {
        auto it = m_Chunks.find(chunkPosition);
        if (it != m_Chunks.end())
        {
            return it->second.getBlock(x, y, z);
        }
        return nullptr;
    }

    Block* World::getBlock(uint16_t x, uint16_t y, uint16_t z)
    {
        glm::ivec2 chunkPosition = { x / CHUNK_LENGTH, z / CHUNK_WIDTH };
        return getBlock(chunkPosition, x % CHUNK_LENGTH, y, z % CHUNK_WIDTH);
    }

    Chunk* World::getChunk(const glm::ivec2& position)
    {
        auto it = m_Chunks.find(position);
        if (it != m_Chunks.end())
        {
            return &it->second;
        }
        return nullptr;
    }
}