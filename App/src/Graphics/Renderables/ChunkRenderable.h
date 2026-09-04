#pragma once

#include "Common/Memory.h"
#include "Data/Chunk.h"
#include "Graphics/GraphicsContext.h"
#include "Graphics/BlockAtlas.h"
#include "Graphics/Renderable.h"
#include "Graphics/ShaderSystems/TerrainRenderer.h"
#include "Graphics/ShaderSystems/WireframeRenderer.h"

#include <webgpu/webgpu_cpp.h>
#include <vector>
#include <glm/glm.hpp>

namespace Minecraft::Graphics
{
    class ChunkRenderable : public TerrainRenderable, public WireframeRenderable
    {
    public:
        ChunkRenderable(Data::Chunk &chunk, GraphicsContext &graphicsContext, const BlockAtlas &blockAtlas);
        ~ChunkRenderable();

        ChunkRenderable(const ChunkRenderable &) = delete;
        ChunkRenderable &operator=(const ChunkRenderable &) = delete;
        ChunkRenderable(ChunkRenderable &&) = delete;
        ChunkRenderable &operator=(ChunkRenderable &&) = delete;

        virtual void Render(wgpu::RenderPassEncoder &renderPass, glm::vec3 cameraPosition) override;
        virtual void RenderWireframe(wgpu::RenderPassEncoder &renderPass, glm::vec3 cameraPosition) override;

        struct ChunkUniforms {
            glm::mat4 transform;
        };

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

        struct CubeUVs
        {
            glm::vec2 topMax;
            glm::vec2 topMin;
            glm::vec2 bottomMax;
            glm::vec2 bottomMin;
            glm::vec2 sideMax;
            glm::vec2 sideMin;
        };

        void BuildMesh();
        void CreateCube(std::vector<TerrainVertex> &vertices,
                        std::vector<uint32_t> &indices,
                        std::vector<WireframeVertex> &wireframeVertices,
                        std::vector<uint32_t> &wireFrameIndices,
                        uint32_t x, uint32_t y, uint32_t z,
                        CubeUVs uvCoord,
                        ActiveFaces activeFaces = ActiveFaces());

        void AppendQuadIndices(std::vector<uint32_t> &indices,
                               std::vector<uint32_t> &wireFrameIndices,
                               uint32_t faceBase,
                               bool invertWindingOrder = false);

        void InitBuffers();
        void InitBindGroup();

    private:
        GraphicsContext &m_GraphicsContext;
        const BlockAtlas &m_BlockAtlas;
        Data::Chunk &m_Chunk;
        wgpu::Buffer m_VertexBuffer;
        wgpu::Buffer m_IndexBuffer;
        wgpu::Buffer m_WireframeVertexBuffer;
        wgpu::Buffer m_WireframeIndexBuffer;
        wgpu::Buffer m_UniformBuffer;
        wgpu::BindGroup m_ChunkBindGroup;
        wgpu::BindGroupLayout m_ChunkBindGroupLayout;

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