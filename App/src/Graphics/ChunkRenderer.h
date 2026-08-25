#pragma once

#include "Common/Memory.h"
#include "Data/Chunk.h"
#include "Graphics/GraphicsContext.h"

#include <webgpu/webgpu_cpp.h>

namespace Minecraft::Graphics
{
    class ChunkRenderer
    {
    public:
        ChunkRenderer(Data::Chunk &chunk, GraphicsContext &graphicsContext);
        ~ChunkRenderer();

        void Render(wgpu::RenderPassEncoder encoder);
        #ifdef MC_DEBUG
        void RenderWireframe(wgpu::RenderPassEncoder encoder);
        #endif

    private:
        void BuildMesh();
        void InitBuffers();

    private:
        GraphicsContext &m_GraphicsContext;
        Data::Chunk &m_Chunk;
        wgpu::Buffer m_VertexBuffer;
        wgpu::Buffer m_IndexBuffer;
        #ifdef MC_DEBUG
        wgpu::Buffer m_WireframeIndexBuffer;
        #endif
        uint32_t m_VertexCount = 0;
        uint32_t m_AllocatedVertexCount = 0;
        uint32_t m_IndexCount = 0;
        uint32_t m_AllocatedIndexCount = 0;
        #ifdef MC_DEBUG
        uint32_t m_WireframeIndexCount = 0;
        uint32_t m_AllocatedWireframeIndexCount = 0;
        #endif
    };
}