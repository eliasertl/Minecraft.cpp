#pragma once

#include "Graphics/GraphicsContext.h"
#include "Graphics/Renderable.h"
#include "Graphics/ShaderSystems/TerrainRenderer.h"
#include "Graphics/ShaderSystems/WireframeRenderer.h"
#include "Common/Memory.h"
#include "Data/Camera.h"
#include "Data/World.h"
#include "Data/Camera.h"
#include "ChunkRenderable.h"

namespace Minecraft::Graphics
{
    class WorldRenderable : public TerrainRenderable, public WireframeRenderable
    {
    public:
        WorldRenderable (Data::World& world, GraphicsContext& graphicsContext, const BlockAtlas& blockAtlas);
        ~WorldRenderable() override;

        virtual void Render(wgpu::RenderPassEncoder& encoder, glm::vec3 cameraPosition) override;
        virtual void RenderWireframe(wgpu::RenderPassEncoder& encoder, glm::vec3 cameraPosition) override;
    
    private:
        void CheckChunkRenderables();

    private:
        std::map<glm::ivec2, Scope<ChunkRenderable>, IVec2Less> m_ChunkRenderables;
        Data::World& m_World;
        const BlockAtlas& m_BlockAtlas;
        GraphicsContext& m_GraphicsContext;
    };
}