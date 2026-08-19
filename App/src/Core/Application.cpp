#include "Application.h"
#include "Logger.h"
#include "Common/File.h"

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

        InitPipeline();
    }

    Application::~Application()
    {
        LOG_INFO("Shutting down Application");
        m_RenderPipeline = nullptr;
        m_GraphicsContext.release();
        m_Window.release();
    }

    void Application::InitPipeline()
    {
        std::string shaderSource = ReadFileToString("assets/shader/main.wgsl");
        
        wgpu::RenderPipelineDescriptor pipelineDesc = {};
        pipelineDesc.nextInChain = nullptr;

        // Shader Module
        wgpu::ShaderModuleDescriptor shaderModuleDesc = {};
        wgpu::ShaderSourceWGSL shaderCodeDesc = {};
        shaderCodeDesc.code = shaderSource.c_str();
        shaderCodeDesc.nextInChain = nullptr;
        shaderCodeDesc.sType = wgpu::SType::ShaderSourceWGSL;
        shaderModuleDesc.nextInChain = &shaderCodeDesc;
        wgpu::ShaderModule shaderModule = m_GraphicsContext->GetDevice().CreateShaderModule(&shaderModuleDesc);


        // Vertex state
        pipelineDesc.vertex.bufferCount = 0;
        pipelineDesc.vertex.buffers = nullptr;
        pipelineDesc.vertex.module = shaderModule;
        pipelineDesc.vertex.entryPoint = "vs_main";
        pipelineDesc.vertex.constantCount = 0;
        pipelineDesc.vertex.constants = nullptr;

        // Fragment state
        wgpu::FragmentState fragmentState = {};
        fragmentState.module = shaderModule;
        fragmentState.entryPoint = "fs_main";
        fragmentState.constantCount = 0;
        fragmentState.constants = nullptr;

        // Blend state
        wgpu::BlendState blendState = {};
        blendState.color.srcFactor = wgpu::BlendFactor::SrcAlpha;
        blendState.color.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
        blendState.color.operation = wgpu::BlendOperation::Add;
        blendState.alpha.srcFactor = wgpu::BlendFactor::Zero;
        blendState.alpha.dstFactor = wgpu::BlendFactor::One;
        blendState.alpha.operation = wgpu::BlendOperation::Add;

        wgpu::ColorTargetState colorTargetState = {};
        colorTargetState.format = m_GraphicsContext->GetSurfaceFormat();
        colorTargetState.blend = &blendState;
        colorTargetState.writeMask = wgpu::ColorWriteMask::All;

        fragmentState.targetCount = 1;
        fragmentState.targets = &colorTargetState;

        pipelineDesc.fragment = &fragmentState;

        // Primitive state
        pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        pipelineDesc.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;
        pipelineDesc.primitive.frontFace = wgpu::FrontFace::CCW;
        pipelineDesc.primitive.cullMode = wgpu::CullMode::None;

        // Multisample state
        pipelineDesc.multisample.count = 1;
        pipelineDesc.multisample.mask = ~0u;
        pipelineDesc.multisample.alphaToCoverageEnabled = false;

        // Depth stencil state
        pipelineDesc.depthStencil = nullptr;

        // Pipeline layout
        pipelineDesc.layout = nullptr;

        m_RenderPipeline = m_GraphicsContext->GetDevice().CreateRenderPipeline(&pipelineDesc);
        shaderModule = nullptr;
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
            renderPassEncoder.SetPipeline(m_RenderPipeline);
            renderPassEncoder.Draw(3, 1, 0, 0);
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