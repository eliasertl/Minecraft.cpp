#pragma once

#include "Common/Memory.h"
#include "Data/Chunk.h"
#include "Graphics/GraphicsContext.h"
#include "Graphics/BlockAtlas.h"

#include <webgpu/webgpu_cpp.h>
#include <vector>
#include <glm/glm.hpp>

namespace Minecraft::Graphics
{
    class ChunkRenderer
    {
    public:
        ChunkRenderer(Data::Chunk &chunk, GraphicsContext &graphicsContext, const BlockAtlas& blockAtlas);
        ~ChunkRenderer();

        void Render(wgpu::RenderPassEncoder encoder);
#ifdef MC_DEBUG
        void RenderWireframe(wgpu::RenderPassEncoder encoder);

#endif

    private:
        struct Vertex
        {
            glm::vec3 position;
            glm::vec2 uv;
        };
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
        void CreateCube(std::vector<Vertex> &vertices,
                        std::vector<uint32_t> &indices,
#ifdef MC_DEBUG
                        std::vector<uint32_t> &wireFrameIndices,
#endif
                        uint32_t x, uint32_t y, uint32_t z,
                        BlockAtlasCoord uvCoord,
                        ActiveFaces activeFaces = ActiveFaces());

        void AppendQuadIndices(std::vector<uint32_t> &indices,
#ifdef MC_DEBUG
                               std::vector<uint32_t> &wireFrameIndices,
#endif
                               uint32_t faceBase);

        void InitBuffers();

    private:
        GraphicsContext &m_GraphicsContext;
        const BlockAtlas& m_BlockAtlas;
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