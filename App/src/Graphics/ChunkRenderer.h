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

    private:
        void BuildMesh();

    private:
        GraphicsContext &m_GraphicsContext;
        Data::Chunk &m_Chunk;
        wgpu::Buffer m_VertexBuffer;
        wgpu::Buffer m_IndexBuffer;
        uint32_t m_VertexCount;
        uint32_t m_IndexCount;
    };
}