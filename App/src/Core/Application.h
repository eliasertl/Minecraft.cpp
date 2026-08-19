#pragma once

#include "Common/Memory.h"
#include "Graphics/Window.h"
#include "Graphics/GraphicsContext.h"

#include <glm/glm.hpp>

namespace Minecraft::Core
{
    class Application
    {
    public:
        Application();
        ~Application();

        void Run();

    private:
        void InitPipeline();
        void InitBuffers();

    private:
        Scope<Graphics::Window> m_Window;
        Scope<Graphics::GraphicsContext> m_GraphicsContext;
        wgpu::RenderPipeline m_RenderPipeline;
        wgpu::Buffer m_VertexBuffer;
        wgpu::Buffer m_IndexBuffer;
        wgpu::Buffer m_CameraUniformBuffer;
        wgpu::BindGroup m_CameraBindGroup;
        wgpu::BindGroupLayout m_CameraBindGroupLayout;
        wgpu::PipelineLayout m_PipelineLayout;

        glm::mat4 m_Transform;
        glm::mat4 m_ViewProjection;

        uint32_t m_VertexCount;
        uint32_t m_IndexCount;
    };
}