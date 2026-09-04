#pragma once

#include "Common/Memory.h"
#include "Graphics/Window.h"
#include "Graphics/GraphicsContext.h"
#include "Graphics/ImGuiContext.h"
#include "Graphics/Renderables/WorldRenderable.h"
#include "Graphics/ShaderSystems/TerrainRenderer.h"
#include "Graphics/ShaderSystems/WireframeRenderer.h"
#include "Graphics/BlockAtlas.h"
#include "Data/BlockTypes.h"
#include "Data/World.h"
#include "Data/Camera.h"

#include <glm/glm.hpp>
#include <webgpu/webgpu_cpp.h>

namespace Minecraft::Core
{
    class Application
    {
    public:
        Application();
        ~Application();

        void Run();

        static Application &Get() { return *s_Instance; }
        
        struct CameraUniforms
        {
            glm::mat4 viewProjection;
        };

    private:
        void OnFrameStart();
        void OnUI(float deltaTime);
        void OnUpdate(float deltaTime);
        void OnFrameEnd();

        void SetTestChunksRandom();
        void SetTestChunksFlat();
        void SetTestChunksEmpty();
        void SetTestChunksFull();

    private:
        static Application *s_Instance;
        float m_SmoothedDeltaTime = 0.016f;
        wgpu::Buffer m_CameraUniformBuffer;
        Scope<Graphics::Window> m_Window;
        Scope<Graphics::GraphicsContext> m_GraphicsContext;
        Scope<Graphics::ImGuiContext> m_ImGuiContext;
        Scope<Data::World> m_World;
        Scope<Data::Camera> m_Camera;
        Scope<Graphics::WorldRenderable> m_WorldRenderable;
        Scope<Graphics::TerrainRenderer> m_TerrainRenderer;
        Scope<Graphics::WireframeRenderer> m_WireframeRenderer;
        Scope<Graphics::BlockAtlas> m_BlockAtlas;
    };
}