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

        InitBuffers();
        InitPipeline();
    }

    Application::~Application()
    {
        LOG_INFO("Shutting down Application");
        m_RenderPipeline = nullptr;
        m_VertexBuffer = nullptr;
        m_IndexBuffer = nullptr;
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

        wgpu::VertexBufferLayout vertexBufferLayout = {};
        std::vector<wgpu::VertexAttribute> vertexAttribs(2);
        vertexAttribs[0].format = wgpu::VertexFormat::Float32x3;
        vertexAttribs[0].offset = 0;
        vertexAttribs[0].shaderLocation = 0;

        vertexAttribs[1].format = wgpu::VertexFormat::Float32x2;
        vertexAttribs[1].offset = sizeof(float) * 3;
        vertexAttribs[1].shaderLocation = 1;

        vertexBufferLayout.attributeCount = vertexAttribs.size();
        vertexBufferLayout.attributes = vertexAttribs.data();
        vertexBufferLayout.arrayStride = sizeof(float) * 5;
        vertexBufferLayout.stepMode = wgpu::VertexStepMode::Vertex;

        pipelineDesc.vertex.bufferCount = 1;
        pipelineDesc.vertex.buffers = &vertexBufferLayout;

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

    void Application::InitBuffers()
    {
        // Vertex buffer
        struct Vertex
        {
            float position[3];
            float uv[2];
        };

        Vertex vertices[] = {
            {{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f}},
            {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}},
            {{0.5f, -0.5f, 0.0f}, {1.0f, 0.0f}},
            {{-0.5f, 0.5f, 0.0f}, {0.0f, 1.0f}}};

        m_VertexCount = static_cast<uint32_t>(sizeof(vertices) / sizeof(Vertex));

        wgpu::BufferDescriptor vertexBufferDesc = {};
        vertexBufferDesc.size = sizeof(vertices);
        vertexBufferDesc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
        vertexBufferDesc.mappedAtCreation = false;
        m_VertexBuffer = m_GraphicsContext->GetDevice().CreateBuffer(&vertexBufferDesc);

        m_GraphicsContext->GetQueue().WriteBuffer(m_VertexBuffer, 0, vertices, sizeof(vertices));
    
        // Index Buffer
        uint16_t indices[] = {
            0, 1, 2,
            0, 3, 1};
        m_IndexCount = static_cast<uint32_t>(sizeof(indices) / sizeof(uint16_t));
        wgpu::BufferDescriptor indexBufferDesc = {};
        indexBufferDesc.size = sizeof(indices);
        indexBufferDesc.usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst;
        indexBufferDesc.mappedAtCreation = false;
        m_IndexBuffer = m_GraphicsContext->GetDevice().CreateBuffer(&indexBufferDesc);

        m_GraphicsContext->GetQueue().WriteBuffer(m_IndexBuffer, 0, indices, sizeof(indices));
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
            renderPassEncoder.SetVertexBuffer(0, m_VertexBuffer);
            renderPassEncoder.SetIndexBuffer(m_IndexBuffer, wgpu::IndexFormat::Uint16);
            renderPassEncoder.DrawIndexed(m_IndexCount, 1, 0, 0, 0);
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