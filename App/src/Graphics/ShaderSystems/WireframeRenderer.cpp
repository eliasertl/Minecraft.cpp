#include "WireframeRenderer.h"
#include "Common/File.h"
#include "Core/Application.h"
#include "Core/Logger.h"

#include <tracy/Tracy.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Minecraft::Graphics
{
    WireframeRenderer::WireframeRenderer(GraphicsContext &graphicsContext, wgpu::Buffer cameraUniformBuffer)
        : m_GraphicsContext(graphicsContext), m_CameraUniformBuffer(cameraUniformBuffer)
    {
        InitPipeline();
    }

    WireframeRenderer::~WireframeRenderer()
    {
        m_RenderPipeline = nullptr;
        m_CameraBindGroup = nullptr;
    }

    void WireframeRenderer::InitPipeline()
    {
        std::string shaderSource = ReadFileToString("assets/shader/wireframe.wgsl");

        wgpu::RenderPipelineDescriptor pipelineDesc = {};
        pipelineDesc.nextInChain = nullptr;
        pipelineDesc.label = "[WireframeRenderer] Pipeline";

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

        vertexAttribs[1].format = wgpu::VertexFormat::Float32x3;
        vertexAttribs[1].offset = sizeof(float) * 3;
        vertexAttribs[1].shaderLocation = 1;

        vertexBufferLayout.attributeCount = vertexAttribs.size();
        vertexBufferLayout.attributes = vertexAttribs.data();
        vertexBufferLayout.arrayStride = sizeof(float) * 6;
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
        pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::LineList;
        pipelineDesc.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;
        pipelineDesc.primitive.frontFace = wgpu::FrontFace::CCW;
        pipelineDesc.primitive.cullMode = wgpu::CullMode::Back;

        // Multisample state
        pipelineDesc.multisample.count = 1;
        pipelineDesc.multisample.mask = ~0u;
        pipelineDesc.multisample.alphaToCoverageEnabled = false;

        // Depth stencil state
        wgpu::DepthStencilState depthStencilState = {};
        depthStencilState.format = m_GraphicsContext.GetDepthFormat();
        depthStencilState.depthWriteEnabled = true;
        depthStencilState.depthCompare = wgpu::CompareFunction::LessEqual;
        pipelineDesc.depthStencil = &depthStencilState;

        // Pipeline layout
        std::vector<wgpu::BindGroupLayoutEntry> bindingLayouts(1);
        bindingLayouts[0].binding = 0;
        bindingLayouts[0].visibility = wgpu::ShaderStage::Vertex;
        bindingLayouts[0].buffer.type = wgpu::BufferBindingType::Uniform;
        bindingLayouts[0].buffer.minBindingSize = sizeof(Core::Application::CameraUniforms);

        wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc = wgpu::BindGroupLayoutDescriptor{};
        bindGroupLayoutDesc.entryCount = bindingLayouts.size();
        bindGroupLayoutDesc.entries = bindingLayouts.data();
        bindGroupLayoutDesc.nextInChain = nullptr;
        m_CameraBindGroupLayout = m_GraphicsContext.GetDevice().CreateBindGroupLayout(&bindGroupLayoutDesc);

        wgpu::PipelineLayoutDescriptor pipelineLayoutDesc = {};
        pipelineLayoutDesc.nextInChain = nullptr;
        pipelineLayoutDesc.bindGroupLayoutCount = 1;
        pipelineLayoutDesc.bindGroupLayouts = &m_CameraBindGroupLayout;
        m_PipelineLayout = m_GraphicsContext.GetDevice().CreatePipelineLayout(&pipelineLayoutDesc);
        pipelineDesc.layout = m_PipelineLayout;

        std::vector<wgpu::BindGroupEntry> bindGroupEntries(1);
        bindGroupEntries[0].binding = 0;
        bindGroupEntries[0].buffer = m_CameraUniformBuffer;
        bindGroupEntries[0].offset = 0;
        bindGroupEntries[0].size = sizeof(Core::Application::CameraUniforms);

        wgpu::BindGroupDescriptor bindGroupDesc = {};
        bindGroupDesc.layout = m_CameraBindGroupLayout;
        bindGroupDesc.entryCount = bindGroupEntries.size();
        bindGroupDesc.entries = bindGroupEntries.data();

        m_CameraBindGroup = m_GraphicsContext.GetDevice().CreateBindGroup(&bindGroupDesc);
        m_RenderPipeline = m_GraphicsContext.GetDevice().CreateRenderPipeline(&pipelineDesc);
        shaderModule = nullptr;

        if (!m_RenderPipeline)
        {
            LOG_ERROR("Failed to create render pipeline!");
        }
    }

    void WireframeRenderer::Render(std::vector<ChunkRenderer *> &renderers, wgpu::RenderPassEncoder encoder)
    {
        ZoneScoped;
        encoder.PushDebugGroup("WireframeRenderer::Render");
        encoder.SetPipeline(m_RenderPipeline);
        encoder.SetBindGroup(0, m_CameraBindGroup);
        for (auto renderer : renderers)
        {
            renderer->RenderWireframe(encoder);
        }
        encoder.PopDebugGroup();
    }
}