#include "WorldRenderable.h"
#include "Core/Logger.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#include <algorithm>

namespace Minecraft::Graphics
{

    WorldRenderable::WorldRenderable(Data::World &world, GraphicsContext &graphicsContext, const BlockAtlas &blockAtlas)
        : m_World(world), m_GraphicsContext(graphicsContext), m_BlockAtlas(blockAtlas)
    {
    }

    WorldRenderable::~WorldRenderable()
    {
    }

    void WorldRenderable::CheckChunkRenderables()
    {
        // Add missing
        for (auto &[position, chunk] : m_World.getChunks())
        {
            if (m_ChunkRenderables.find(position) == m_ChunkRenderables.end())
            {
                m_ChunkRenderables.emplace(position, CreateScope<ChunkRenderable>(chunk, m_GraphicsContext, m_BlockAtlas));
            }
        }

        // Remove extra
        for (auto it = m_ChunkRenderables.begin(); it != m_ChunkRenderables.end();)
        {
            if (m_World.getChunks().find(it->first) == m_World.getChunks().end())
            {
                it = m_ChunkRenderables.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    void WorldRenderable::Render(wgpu::RenderPassEncoder &encoder, glm::vec3 cameraPosition)
    {
        CheckChunkRenderables();

        std::vector<std::pair<glm::ivec2, ChunkRenderable *>> sortedChunks;
        sortedChunks.reserve(m_ChunkRenderables.size());

        for (auto &[position, ChunkRenderable] : m_ChunkRenderables)
        {
            sortedChunks.emplace_back(position, ChunkRenderable.get());
        }

        std::sort(sortedChunks.begin(), sortedChunks.end(),
                  [&cameraPosition](const auto &a, const auto &b)
                  {
                      glm::vec3 centerA = {
                          (a.first.x + 0.5f) * Data::CHUNK_LENGTH,
                          0.0f,
                          (a.first.y + 0.5f) * Data::CHUNK_WIDTH};

                      glm::vec3 centerB = {
                          (b.first.x + 0.5f) * Data::CHUNK_LENGTH,
                          0.0f,
                          (b.first.y + 0.5f) * Data::CHUNK_WIDTH};

                      float distanceA = glm::length2(centerA - cameraPosition);
                      float distanceB = glm::length2(centerB - cameraPosition);

                      return distanceA < distanceB;
                  });

        for (auto &[position, ChunkRenderable] : sortedChunks)
        {
            ChunkRenderable->Render(encoder, cameraPosition);
        }
    }

    void WorldRenderable::RenderWireframe(wgpu::RenderPassEncoder &encoder, glm::vec3 cameraPosition)
    {
        CheckChunkRenderables();

        std::vector<std::pair<glm::ivec2, ChunkRenderable *>> sortedChunks;
        sortedChunks.reserve(m_ChunkRenderables.size());

        for (auto &[position, ChunkRenderable] : m_ChunkRenderables)
        {
            sortedChunks.emplace_back(position, ChunkRenderable.get());
        }

        std::sort(sortedChunks.begin(), sortedChunks.end(),
                  [&cameraPosition](const auto &a, const auto &b)
                  {
                      glm::vec3 centerA = {
                          (a.first.x + 0.5f) * Data::CHUNK_LENGTH,
                          0.0f,
                          (a.first.y + 0.5f) * Data::CHUNK_WIDTH};

                      glm::vec3 centerB = {
                          (b.first.x + 0.5f) * Data::CHUNK_LENGTH,
                          0.0f,
                          (b.first.y + 0.5f) * Data::CHUNK_WIDTH};

                      float distanceA = glm::length2(centerA - cameraPosition);
                      float distanceB = glm::length2(centerB - cameraPosition);

                      return distanceA < distanceB;
                  });

        for (auto &[position, ChunkRenderable] : sortedChunks)
        {
            ChunkRenderable->RenderWireframe(encoder, cameraPosition);
        }
    }
}