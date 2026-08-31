#pragma once

#include "Graphics/GraphicsContext.h"
#include "Data/Camera.h"
#include <vector>
#include <webgpu/webgpu_cpp.h>
#include <glm/glm.hpp>

namespace Minecraft::Graphics
{
    struct WireframeVertex
    {
        glm::vec3 position;
        glm::vec3 color;
    };

    struct WorldRenderer;

    class WireframeRenderer
    {
    public:
        WireframeRenderer(GraphicsContext& graphicsContext, wgpu::Buffer cameraUniformBuffer);
        ~WireframeRenderer();

        void Render(WorldRenderer* renderer, wgpu::RenderPassEncoder encoder);

    private:
        void InitPipeline();

    private:
        GraphicsContext& m_GraphicsContext;
        wgpu::RenderPipeline m_RenderPipeline;
        wgpu::Buffer m_CameraUniformBuffer;
        wgpu::BindGroup m_BindGroup;
        wgpu::BindGroupLayout m_BindGroupLayout;
        wgpu::BindGroupLayout m_ChunkBindGroupLayout;
        wgpu::PipelineLayout m_PipelineLayout;

        glm::mat4 m_Transform;
        glm::mat4 m_ViewProjection;
    };
}