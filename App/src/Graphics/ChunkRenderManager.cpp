#include "ChunkRenderManager.h"
#include "Common/File.h"
#include "Core/Logger.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Minecraft::Graphics
{
    struct CameraUniforms
    {
        glm::mat4 viewProjection;
        glm::mat4 transform;
    };

    ChunkRenderManager::ChunkRenderManager(GraphicsContext &graphicsContext)
        : m_GraphicsContext(graphicsContext)
    {
        InitCameraBuffer();
        InitPipeline();
    }

    ChunkRenderManager::~ChunkRenderManager()
    {
        m_RenderPipeline = nullptr;
        m_CameraBindGroup = nullptr;
    }

    void ChunkRenderManager::InitPipeline()
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
        wgpu::ShaderModule shaderModule = m_GraphicsContext.GetDevice().CreateShaderModule(&shaderModuleDesc);

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
        colorTargetState.format = m_GraphicsContext.GetSurfaceFormat();
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
        wgpu::DepthStencilState depthStencilState = {};
        depthStencilState.format = m_GraphicsContext.GetDepthFormat();
        depthStencilState.depthWriteEnabled = true;
        depthStencilState.depthCompare = wgpu::CompareFunction::Less;
        pipelineDesc.depthStencil = &depthStencilState;

        // Pipeline layout
        wgpu::BindGroupLayoutEntry bindingLayout = {};
        bindingLayout.binding = 0;
        bindingLayout.visibility = wgpu::ShaderStage::Vertex;
        bindingLayout.buffer.type = wgpu::BufferBindingType::Uniform;
        bindingLayout.buffer.minBindingSize = sizeof(CameraUniforms);

        wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc = wgpu::BindGroupLayoutDescriptor{};
        bindGroupLayoutDesc.entryCount = 1;
        bindGroupLayoutDesc.entries = &bindingLayout;
        bindGroupLayoutDesc.nextInChain = nullptr;
        m_CameraBindGroupLayout = m_GraphicsContext.GetDevice().CreateBindGroupLayout(&bindGroupLayoutDesc);

        wgpu::PipelineLayoutDescriptor pipelineLayoutDesc = {};
        pipelineLayoutDesc.nextInChain = nullptr;
        pipelineLayoutDesc.bindGroupLayoutCount = 1;
        pipelineLayoutDesc.bindGroupLayouts = &m_CameraBindGroupLayout;
        m_PipelineLayout = m_GraphicsContext.GetDevice().CreatePipelineLayout(&pipelineLayoutDesc);
        pipelineDesc.layout = m_PipelineLayout;

        wgpu::BindGroupEntry bindGroupEntry = {};
        bindGroupEntry.binding = 0;
        bindGroupEntry.buffer = m_CameraUniformBuffer;
        bindGroupEntry.offset = 0;
        bindGroupEntry.size = sizeof(CameraUniforms);

        wgpu::BindGroupDescriptor bindGroupDesc = {};
        bindGroupDesc.layout = m_CameraBindGroupLayout;
        bindGroupDesc.entryCount = 1;
        bindGroupDesc.entries = &bindGroupEntry;

        m_CameraBindGroup = m_GraphicsContext.GetDevice().CreateBindGroup(&bindGroupDesc);

        m_RenderPipeline = m_GraphicsContext.GetDevice().CreateRenderPipeline(&pipelineDesc);
        shaderModule = nullptr;

        if (!m_RenderPipeline)
        {
            LOG_ERROR("Failed to create render pipeline!");
        }
    }

    void ChunkRenderManager::InitCameraBuffer()
    {
        wgpu::BufferDescriptor cameraUniformBufferDesc = {};
        cameraUniformBufferDesc.size = sizeof(CameraUniforms);
        cameraUniformBufferDesc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
        cameraUniformBufferDesc.mappedAtCreation = false;
        m_CameraUniformBuffer = m_GraphicsContext.GetDevice().CreateBuffer(&cameraUniformBufferDesc);

        m_Transform = glm::mat4(1.0f);
        float distance = Data::CHUNK_LENGTH + 3.0f;
        m_ViewProjection = glm::perspective(glm::radians(45.0f), static_cast<float>(m_GraphicsContext.GetWindow().GetWidth()) / static_cast<float>(m_GraphicsContext.GetWindow().GetHeight()), 0.1f, 100.0f) * glm::lookAt(glm::vec3(distance), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    void ChunkRenderManager::Render(std::vector<ChunkRenderer *> &renderers, wgpu::RenderPassEncoder encoder)
    {
        CameraUniforms cameraUniforms;
        cameraUniforms.viewProjection = m_ViewProjection;
        cameraUniforms.transform = m_Transform;
        m_GraphicsContext.GetQueue().WriteBuffer(m_CameraUniformBuffer, 0, &cameraUniforms, sizeof(CameraUniforms));

        encoder.SetPipeline(m_RenderPipeline);
        encoder.SetBindGroup(0, m_CameraBindGroup);
        for (auto renderer : renderers)
        {
            renderer->Render(encoder);
        }
    }
}