#include "Application.h"
#include "Logger.h"
#include "Common/Input.h"

#include <stdio.h>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>
#include <random>
#include <imgui.h>

namespace Minecraft::Core
{
    struct ApplicationSettings
    {
        #ifdef MC_DEBUG
        bool renderWireframe = false;
        #endif
    };

    static ApplicationSettings s_ApplicationSettings;

    Application::Application()
    {
        Log::Init();
        LOG_INFO("Creating Application");

        Graphics::WindowProps winProps;
        winProps.title = "Minecraft.cpp";
        winProps.width = 1280;
        winProps.height = 720;
        m_Window = CreateScope<Graphics::Window>(winProps);
        m_Window->Maximize();
        m_GraphicsContext = CreateScope<Graphics::GraphicsContext>(*m_Window);
        m_Camera = CreateScope<Data::Camera>(*m_Window);
        m_Camera->SetViewSize(static_cast<float>(m_Window->GetWidth()), static_cast<float>(m_Window->GetHeight()));
        m_Camera->SetTransform(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, Data::CHUNK_LENGTH * 1.5f)));
        m_ImGuiContext = CreateScope<Graphics::ImGuiContext>(*m_Window, m_GraphicsContext->GetDevice(), m_GraphicsContext->GetQueue(), m_GraphicsContext->GetSurfaceFormat(), m_GraphicsContext->GetDepthFormat());
        Input::Init(m_Window.get());

        m_TestChunk = CreateScope<Data::Chunk>();
        auto gen = std::bind(std::uniform_int_distribution<>(0, 1), std::default_random_engine());
        for (uint16_t x = 0; x < Data::CHUNK_LENGTH; x++)
        {
            for (uint16_t y = 0; y < Data::CHUNK_LENGTH; y++)
            {
                for (uint16_t z = 0; z < Data::CHUNK_LENGTH; z++)
                {
                    m_TestChunk->setBlock(x, y, z, gen(), 0);
                }
            }
        }
        m_TestChunk->markDirty();

        m_TestChunkRenderer = CreateScope<Graphics::ChunkRenderer>(*m_TestChunk, *m_GraphicsContext);
        m_ChunkRenderManager = CreateScope<Graphics::ChunkRenderManager>(*m_GraphicsContext);
    }

    Application::~Application()
    {
        LOG_INFO("Shutting down Application");
        m_TestChunkRenderer.release();
        m_TestChunk.release();
        m_ImGuiContext.release();
        m_GraphicsContext.release();
        m_Window.release();
    }

    void Application::OnFrameStart()
    {
        float deltaTime = m_Window->GetDeltaTime();
        Input::Update();
        m_Window->Update();
        m_Camera->Update(deltaTime);
        m_ImGuiContext->NewFrame();
    }

    void Application::OnUI(float deltaTime)
    {
        ImGui::Begin("Info");
        ImGui::Text("FOV: %.2f", glm::degrees(m_Camera->GetFOV()));
        ImGui::Text("Width: %.2f, Height: %.2f", m_Camera->GetViewSize().x, m_Camera->GetViewSize().y);
        ImGui::Text("Position: (%.2f, %.2f, %.2f)", m_Camera->GetTransform()[3][0], m_Camera->GetTransform()[3][1], m_Camera->GetTransform()[3][2]);
        ImGui::Separator();
        ImGui::Text("FPS: %.2f", 1.0f / deltaTime);
        ImGui::Text("Frame Time: %.2f ms", deltaTime * 1000.0f);
        ImGui::Text("Window Size: %d x %d", m_Window->GetWidth(), m_Window->GetHeight());
        ImGui::Separator();
        #ifdef MC_DEBUG
        ImGui::Checkbox("Render Wireframe", &s_ApplicationSettings.renderWireframe);
        #endif
        ImGui::End();
    }

    void Application::OnUpdate(float deltaTime)
    {
        auto [texture, view] = m_GraphicsContext->AcquireNextTexture();

        wgpu::RenderPassDescriptor renderPassDesc = {};
        renderPassDesc.nextInChain = nullptr;

        wgpu::RenderPassColorAttachment renderPassColorAttachment = {};
        renderPassColorAttachment.view = view;
        renderPassColorAttachment.resolveTarget = nullptr;
        renderPassColorAttachment.loadOp = wgpu::LoadOp::Clear;
        renderPassColorAttachment.storeOp = wgpu::StoreOp::Store;
        renderPassColorAttachment.clearValue = wgpu::Color{0.1, 0.2, 0.8, 1.0};
        renderPassColorAttachment.depthSlice = wgpu::kDepthSliceUndefined;

        renderPassDesc.colorAttachmentCount = 1;
        renderPassDesc.colorAttachments = &renderPassColorAttachment;
        renderPassDesc.depthStencilAttachment = nullptr;
        renderPassDesc.timestampWrites = nullptr;

        wgpu::RenderPassDepthStencilAttachment depthStencilAttachment = {};
        depthStencilAttachment.view = m_GraphicsContext->GetDepthTextureView();
        depthStencilAttachment.depthLoadOp = wgpu::LoadOp::Clear;
        depthStencilAttachment.depthStoreOp = wgpu::StoreOp::Store;
        depthStencilAttachment.depthClearValue = 1.0f;

        renderPassDesc.depthStencilAttachment = &depthStencilAttachment;

        wgpu::CommandEncoder encoder = m_GraphicsContext->GetDevice().CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPassEncoder = encoder.BeginRenderPass(&renderPassDesc);
        std::vector<Graphics::ChunkRenderer *> renderers = {m_TestChunkRenderer.get()};
#ifdef MC_DEBUG
        if (s_ApplicationSettings.renderWireframe)
        {
            m_ChunkRenderManager->RenderWireframe(renderers, *m_Camera, renderPassEncoder);
        }
        else
        {
            m_ChunkRenderManager->Render(renderers, *m_Camera, renderPassEncoder);
        }
#else
m_ChunkRenderManager->Render(renderers, *m_Camera, renderPassEncoder);
#endif
        
        m_ImGuiContext->Render(renderPassEncoder);
        renderPassEncoder.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        m_GraphicsContext->GetDevice().GetQueue().Submit(1, &commands);

        renderPassEncoder = nullptr;

        view = nullptr;
        m_GraphicsContext->GetSurface().Present();

        m_GraphicsContext->GetDevice().Tick();
    }

    void Application::OnFrameEnd()
    {
    }

    void Application::Run()
    {
        LOG_INFO("Running Application");
        while (!m_Window->ShouldClose())
        {
            OnFrameStart();
            OnUI(m_Window->GetDeltaTime());
            OnUpdate(m_Window->GetDeltaTime());
            OnFrameEnd();
        }
    }
}