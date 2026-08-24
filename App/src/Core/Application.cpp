#include "Application.h"
#include "Logger.h"

#include <stdio.h>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>
#include <random>

namespace Minecraft::Core
{
    Application::Application()
    {
        Log::Init();
        LOG_INFO("Creating Application");

        Graphics::WindowProps winProps;
        winProps.title = "Minecraft.cpp";
        winProps.width = 1280;
        winProps.height = 720;
        m_Window = CreateScope<Graphics::Window>(winProps);

        m_GraphicsContext = CreateScope<Graphics::GraphicsContext>(*m_Window);

        m_ImGuiContext = CreateScope<Graphics::ImGuiContext>(*m_Window, m_GraphicsContext->GetDevice(), m_GraphicsContext->GetQueue(), m_GraphicsContext->GetSurfaceFormat(), m_GraphicsContext->GetDepthFormat());

        m_TestChunk = CreateScope<Data::Chunk>();
        auto gen = std::bind(std::uniform_int_distribution<>(0,1),std::default_random_engine());
        for(uint16_t x = 0; x < Data::CHUNK_LENGTH; x++)
        {
            for(uint16_t y = 0; y < Data::CHUNK_LENGTH; y++)
            {
                for(uint16_t z = 0; z < Data::CHUNK_LENGTH; z++)
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

    void Application::Run()
    {
        LOG_INFO("Running Application");
        while (!m_Window->ShouldClose())
        {
            m_Window->Update();

            auto [texture, view] = m_GraphicsContext->AcquireNextTexture();
            if (!view)
            {
                LOG_ERROR("Failed to acquire next view");
                continue;
            }

            m_ImGuiContext->NewFrame();

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
            std::vector<Graphics::ChunkRenderer*> renderers = { m_TestChunkRenderer.get() };
            m_ChunkRenderManager->Render(renderers, renderPassEncoder);
            m_ImGuiContext->Render(renderPassEncoder); // TODO: Seperate Render Pass
            renderPassEncoder.End();
            wgpu::CommandBuffer commands = encoder.Finish();
            m_GraphicsContext->GetDevice().GetQueue().Submit(1, &commands);

            renderPassEncoder = nullptr;

            view = nullptr;
            m_GraphicsContext->GetSurface().Present();

            m_GraphicsContext->GetDevice().Tick();
        }
    }
}