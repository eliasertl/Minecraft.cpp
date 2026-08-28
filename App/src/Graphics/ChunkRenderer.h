#pragma once

#include "Common/Memory.h"
#include "Data/Chunk.h"
#include "Graphics/GraphicsContext.h"
#include "Graphics/BlockAtlas.h"
#include "Graphics/ShaderSystems/TerrainRenderer.h"
#include "Graphics/ShaderSystems/WireframeRenderer.h"

#include <webgpu/webgpu_cpp.h>
#include <vector>
#include <glm/glm.hpp>

namespace Minecraft::Graphics
{
    class ChunkRenderer
    {
    public:
        ChunkRenderer(Data::Chunk &chunk, GraphicsContext &graphicsContext, const BlockAtlas &blockAtlas);
        ~ChunkRenderer();

        void Render(wgpu::RenderPassEncoder encoder);
        void RenderWireframe(wgpu::RenderPassEncoder encoder);

    private:
        struct ActiveFaces
        {
            bool front = false;
            bool back = false;
            bool right = false;
            bool left = false;
            bool top = false;
            bool bottom = false;
        };

        void BuildMesh();
        void CreateCube(std::vector<TerrainVertex> &vertices,
                        std::vector<uint32_t> &indices,
                        std::vector<WireframeVertex> &wireframeVertices,
                        std::vector<uint32_t> &wireFrameIndices,
                        uint32_t x, uint32_t y, uint32_t z,
                        BlockAtlasCoord uvCoord,
                        ActiveFaces activeFaces = ActiveFaces());

        void AppendQuadIndices(std::vector<uint32_t> &indices,
                               std::vector<uint32_t> &wireFrameIndices,
                               uint32_t faceBase,
                               bool invertWindingOrder = false);

        void InitBuffers();

    private:
        GraphicsContext &m_GraphicsContext;
        const BlockAtlas &m_BlockAtlas;
        Data::Chunk &m_Chunk;
        wgpu::Buffer m_VertexBuffer;
        wgpu::Buffer m_IndexBuffer;
        wgpu::Buffer m_WireframeVertexBuffer;
        wgpu::Buffer m_WireframeIndexBuffer;

        uint32_t m_VertexCount = 0;
        uint32_t m_AllocatedVertexCount = 0;
        uint32_t m_IndexCount = 0;
        uint32_t m_AllocatedIndexCount = 0;
        uint32_t m_WireframeVertexCount = 0;
        uint32_t m_AllocatedWireframeVertexCount = 0;
        uint32_t m_WireframeIndexCount = 0;
        uint32_t m_AllocatedWireframeIndexCount = 0;
    };
}