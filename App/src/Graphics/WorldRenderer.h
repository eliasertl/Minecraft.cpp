#pragma once

#include "Graphics/GraphicsContext.h"
#include "Graphics/ShaderSystems/TerrainRenderer.h"
#include "Graphics/ShaderSystems/WireframeRenderer.h"
#include "Common/Memory.h"
#include "Graphics/ChunkRenderer.h"
#include "Data/Camera.h"
#include "Data/World.h"
#include "Data/Camera.h"

namespace Minecraft::Graphics
{
    class WorldRenderer
    {
    public:
        WorldRenderer (Data::World& world, GraphicsContext& graphicsContext, const BlockAtlas& blockAtlas);
        ~WorldRenderer();

        void Render(wgpu::RenderPassEncoder& encoder);
        void RenderWireframe(wgpu::RenderPassEncoder& encoder);
    
    private:
        void CheckChunkRenderers();

    private:
        std::map<glm::ivec2, Scope<ChunkRenderer>, IVec2Less> m_ChunkRenderers;
        Data::World& m_World;
        const BlockAtlas& m_BlockAtlas;
        GraphicsContext& m_GraphicsContext;
    };
}