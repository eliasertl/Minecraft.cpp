#include "WorldRenderer.h"
#include "Core/Logger.h"

#include <glm/gtc/matrix_transform.hpp>

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

    void WorldRenderer::Render(wgpu::RenderPassEncoder &encoder)
    {
        CheckChunkRenderers();
        for (auto &[position, chunkRenderer] : m_ChunkRenderers)
        {
            chunkRenderer->Render(encoder);
        }
    }

    void WorldRenderer::RenderWireframe(wgpu::RenderPassEncoder &encoder)
    {
        CheckChunkRenderers();
        for (auto &[position, chunkRenderer] : m_ChunkRenderers)
        {
            chunkRenderer->RenderWireframe(encoder);
        }
    }
}