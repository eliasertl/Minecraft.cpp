#pragma once

#include "ChunkRenderer.h"
#include "GraphicsContext.h"
#include <vector>
#include <webgpu/webgpu_cpp.h>
#include <glm/glm.hpp>

namespace Minecraft::Graphics
{
    class ChunkRenderManager
    {
    public:
        ChunkRenderManager(GraphicsContext& graphicsContext);
        ~ChunkRenderManager();

        void Render(std::vector<ChunkRenderer *> &renderers, wgpu::RenderPassEncoder encoder);

    private:
        void InitPipeline();
        void InitCameraBuffer();

    private:
        GraphicsContext& m_GraphicsContext;
        wgpu::RenderPipeline m_RenderPipeline;
        wgpu::Buffer m_CameraUniformBuffer;
        wgpu::BindGroup m_CameraBindGroup;
        wgpu::BindGroupLayout m_CameraBindGroupLayout;
        wgpu::PipelineLayout m_PipelineLayout;

        glm::mat4 m_Transform;
        glm::mat4 m_ViewProjection;
    };
}