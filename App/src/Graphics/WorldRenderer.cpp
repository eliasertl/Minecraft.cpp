#include "WorldRenderer.h"
#include "Core/Logger.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#include <algorithm>

namespace Minecraft::Graphics
{

    WorldRenderer::WorldRenderer(Data::World &world, GraphicsContext &graphicsContext, const BlockAtlas &blockAtlas)
        : m_World(world), m_GraphicsContext(graphicsContext), m_BlockAtlas(blockAtlas)
    {
    }

    WorldRenderer::~WorldRenderer()
    {
    }

    void WorldRenderer::CheckChunkRenderers()
    {
        // Add missing
        for (auto &[position, chunk] : m_World.getChunks())
        {
            if (m_ChunkRenderers.find(position) == m_ChunkRenderers.end())
            {
                m_ChunkRenderers.emplace(position, CreateScope<ChunkRenderer>(chunk, m_GraphicsContext, m_BlockAtlas));
            }
        }

        // Remove extra
        for (auto it = m_ChunkRenderers.begin(); it != m_ChunkRenderers.end();)
        {
            if (m_World.getChunks().find(it->first) == m_World.getChunks().end())
            {
                it = m_ChunkRenderers.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    void WorldRenderer::Render(wgpu::RenderPassEncoder &encoder, const glm::vec3 &cameraPosition)
    {
        CheckChunkRenderers();

        std::vector<std::pair<glm::ivec2, ChunkRenderer *>> sortedChunks;
        sortedChunks.reserve(m_ChunkRenderers.size());

        for (auto &[position, chunkRenderer] : m_ChunkRenderers)
        {
            sortedChunks.emplace_back(position, chunkRenderer.get());
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

        for (auto &[position, chunkRenderer] : sortedChunks)
        {
            chunkRenderer->Render(encoder);
        }
    }

    void WorldRenderer::RenderWireframe(wgpu::RenderPassEncoder &encoder, const glm::vec3 &cameraPosition)
    {
        CheckChunkRenderers();

        std::vector<std::pair<glm::ivec2, ChunkRenderer *>> sortedChunks;
        sortedChunks.reserve(m_ChunkRenderers.size());

        for (auto &[position, chunkRenderer] : m_ChunkRenderers)
        {
            sortedChunks.emplace_back(position, chunkRenderer.get());
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

        for (auto &[position, chunkRenderer] : sortedChunks)
        {
            chunkRenderer->RenderWireframe(encoder);
        }
    }
}