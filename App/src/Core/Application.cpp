#include "Application.h"
#include "Logger.h"

#include <stdio.h>

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
    }

    Application::~Application()
    {
        LOG_INFO("Shutting down Application");
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

            wgpu::CommandEncoder encoder = m_GraphicsContext->GetDevice().CreateCommandEncoder();
            wgpu::RenderPassEncoder renderPassEncoder = encoder.BeginRenderPass(&renderPassDesc);
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