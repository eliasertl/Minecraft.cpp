#pragma once

#include "Common/Memory.h"
#include "Graphics/Window.h"
#include "Graphics/GraphicsContext.h"
#include "Graphics/ImGuiContext.h"
#include "Graphics/ChunkRenderer.h"
#include "Graphics/ChunkRenderManager.h"
#include "Data/Chunk.h"
#include "Data/Camera.h"

#include <glm/glm.hpp>

namespace Minecraft::Core
{
    class Application
    {
    public:
        Application();
        ~Application();

        void Run();

        static Application& Get() { return *s_Instance; }

    private:
        void OnFrameStart();
        void OnUI(float deltaTime);
        void OnUpdate(float deltaTime);
        void OnFrameEnd();

    private:
        static Application* s_Instance;
        Scope<Graphics::Window> m_Window;
        Scope<Graphics::GraphicsContext> m_GraphicsContext;
        Scope<Graphics::ImGuiContext> m_ImGuiContext;
        Scope<Data::Chunk> m_TestChunk;
        Scope<Data::Camera> m_Camera;
        Scope<Graphics::ChunkRenderer> m_TestChunkRenderer;
        Scope<Graphics::ChunkRenderManager> m_ChunkRenderManager;
    };
}