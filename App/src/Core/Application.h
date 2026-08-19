#pragma once

#include "Common/Memory.h"
#include "Graphics/Window.h"
#include "Graphics/GraphicsContext.h"

namespace Minecraft::Core {
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
        uint32_t m_VertexCount;
    };
}