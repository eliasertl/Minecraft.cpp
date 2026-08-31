#pragma once

#include "Chunk.h"
#include "Other/GLMCompare.h"

#include <map>
#include <glm/glm.hpp>

namespace Minecraft::Data
{
    class World
    {
    public:
        World();
        ~World();

        void addChunk(const Chunk& chunk, const glm::ivec2& position);
        void addEmptyChunk(const glm::ivec2& position);
        bool hasChunk(const glm::ivec2& position) const;
        bool removeChunk(const glm::ivec2& position);
        Chunk* getChunk(const glm::ivec2& position);

        // Chunk Positions are in chunk space (e.g. 0, 0 is the first, 1, 0 is the second, etc.)
        Block* getBlock(const glm::ivec2& chunkPosition, uint16_t x, uint16_t y, uint16_t z);
        Block* getBlock(uint16_t x, uint16_t y, uint16_t z);

        inline std::map<glm::ivec2, Chunk, IVec2Less>& getChunks() { return m_Chunks; }
        inline const std::map<glm::ivec2, Chunk, IVec2Less>& getChunks() const { return m_Chunks; }

    private:
        std::map<glm::ivec2, Chunk, IVec2Less> m_Chunks;
    };
}