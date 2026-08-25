#pragma once

#include "ChunkRenderer.h"
#include "GraphicsContext.h"
#include "BlockAtlas.h"
#include "Data/Camera.h"
#include <vector>
#include <webgpu/webgpu_cpp.h>
#include <glm/glm.hpp>

namespace Minecraft::Graphics
{
    class ChunkRenderManager
    {
    public:
        ChunkRenderManager(GraphicsContext& graphicsContext, const BlockAtlas& blockAtlas);
        ~ChunkRenderManager();

        void Render(std::vector<ChunkRenderer *> &renderers, const Data::Camera& camera, wgpu::RenderPassEncoder encoder);
        #ifdef MC_DEBUG
        void RenderWireframe(std::vector<ChunkRenderer *> &renderers, const Data::Camera& camera, wgpu::RenderPassEncoder encoder);
        #endif

    private:
        void InitPipeline();
        void InitCameraBuffer();

    private:
        GraphicsContext& m_GraphicsContext;
        const BlockAtlas& m_BlockAtlas;
        wgpu::RenderPipeline m_RenderPipeline;
        #ifdef MC_DEBUG
        wgpu::RenderPipeline m_RenderWireframePipeline;
        #endif
        wgpu::Buffer m_CameraUniformBuffer;
        wgpu::BindGroup m_CameraBindGroup;
        wgpu::BindGroupLayout m_CameraBindGroupLayout;
        wgpu::PipelineLayout m_PipelineLayout;

        glm::mat4 m_Transform;
        glm::mat4 m_ViewProjection;
    };
}