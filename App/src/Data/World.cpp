#include "World.h"

#include <tracy/Tracy.hpp>

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
        ZoneScoped;
        Chunk storedChunk = chunk;
        storedChunk.m_Position = position;
        storedChunk.m_World = this;
        m_Chunks[position] = storedChunk;
    }

    void World::addEmptyChunk(const glm::ivec2& position)
    {
        ZoneScoped;
        Chunk chunk = Chunk();
        chunk.m_Position = position;
        chunk.m_World = this;
        m_Chunks[position] = chunk;
    }

    bool World::hasChunk(const glm::ivec2& position) const
    {
        return m_Chunks.find(position) != m_Chunks.end();
    }

    bool World::removeChunk(const glm::ivec2& position)
    {
        ZoneScoped;
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

    Block* World::getBlock(int32_t x, int32_t y, int32_t z)
    {
        if (y < 0 || y >= static_cast<int32_t>(CHUNK_HEIGHT))
            return nullptr;

        int32_t chunkX = x / static_cast<int32_t>(CHUNK_LENGTH);
        if (x < 0 && x % static_cast<int32_t>(CHUNK_LENGTH) != 0)
            --chunkX;

        int32_t chunkZ = z / static_cast<int32_t>(CHUNK_WIDTH);
        if (z < 0 && z % static_cast<int32_t>(CHUNK_WIDTH) != 0)
            --chunkZ;

        const uint16_t localX = static_cast<uint16_t>(x - chunkX * static_cast<int32_t>(CHUNK_LENGTH));
        const uint16_t localZ = static_cast<uint16_t>(z - chunkZ * static_cast<int32_t>(CHUNK_WIDTH));

        return getBlock({chunkX, chunkZ}, localX, static_cast<uint16_t>(y), localZ);
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