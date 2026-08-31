#include "TerrainRenderer.h"
#include "Common/File.h"
#include "Core/Application.h"
#include "Core/Logger.h"
#include "Graphics/WorldRenderer.h"

#include <tracy/Tracy.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Minecraft::Graphics
{
    TerrainRenderer::TerrainRenderer(GraphicsContext &graphicsContext, wgpu::Buffer cameraUniformBuffer, const BlockAtlas &blockAtlas)
        : m_GraphicsContext(graphicsContext), m_CameraUniformBuffer(cameraUniformBuffer), m_BlockAtlas(blockAtlas)
    {
        InitPipeline();
    }

    TerrainRenderer::~TerrainRenderer()
    {
        m_RenderPipeline = nullptr;
        m_BindGroup = nullptr;
    }

    void TerrainRenderer::InitPipeline()
    {
        std::string shaderSource = ReadFileToString("assets/shader/terrain.wgsl");

        wgpu::RenderPipelineDescriptor pipelineDesc = {};
        pipelineDesc.nextInChain = nullptr;
        pipelineDesc.label = "[TerrainRenderer] Pipeline";

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
        std::vector<wgpu::VertexAttribute> vertexAttribs(3);
        vertexAttribs[0].format = wgpu::VertexFormat::Float32x3;
        vertexAttribs[0].offset = 0;
        vertexAttribs[0].shaderLocation = 0;

        vertexAttribs[1].format = wgpu::VertexFormat::Float32x3;
        vertexAttribs[1].offset = sizeof(float) * 3;
        vertexAttribs[1].shaderLocation = 1;

        vertexAttribs[2].format = wgpu::VertexFormat::Float32x2;
        vertexAttribs[2].offset = sizeof(float) * 6;
        vertexAttribs[2].shaderLocation = 2;

        vertexBufferLayout.attributeCount = vertexAttribs.size();
        vertexBufferLayout.attributes = vertexAttribs.data();
        vertexBufferLayout.arrayStride = sizeof(float) * 8;
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
        pipelineDesc.primitive.cullMode = wgpu::CullMode::Back;

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
        std::vector<wgpu::BindGroupLayoutEntry> bindingLayouts(3);
        bindingLayouts[0].binding = 0;
        bindingLayouts[0].visibility = wgpu::ShaderStage::Vertex;
        bindingLayouts[0].buffer.type = wgpu::BufferBindingType::Uniform;
        bindingLayouts[0].buffer.minBindingSize = sizeof(Core::Application::CameraUniforms);

        bindingLayouts[1].binding = 1;
        bindingLayouts[1].visibility = wgpu::ShaderStage::Fragment;
        bindingLayouts[1].texture.sampleType = wgpu::TextureSampleType::Float;
        bindingLayouts[1].texture.viewDimension = wgpu::TextureViewDimension::e2D;

        bindingLayouts[2].binding = 2;
        bindingLayouts[2].visibility = wgpu::ShaderStage::Fragment;
        bindingLayouts[2].sampler.type = wgpu::SamplerBindingType::Filtering;

        wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc = wgpu::BindGroupLayoutDescriptor{};
        bindGroupLayoutDesc.entryCount = bindingLayouts.size();
        bindGroupLayoutDesc.entries = bindingLayouts.data();
        bindGroupLayoutDesc.nextInChain = nullptr;
        m_BindGroupLayout = m_GraphicsContext.GetDevice().CreateBindGroupLayout(&bindGroupLayoutDesc);

        std::vector<wgpu::BindGroupLayoutEntry> chunkBindingLayouts(1);
        chunkBindingLayouts[0].binding = 0;
        chunkBindingLayouts[0].visibility = wgpu::ShaderStage::Vertex;
        chunkBindingLayouts[0].buffer.type = wgpu::BufferBindingType::Uniform;
        chunkBindingLayouts[0].buffer.minBindingSize = sizeof(Graphics::ChunkRenderer::ChunkUniforms);

        wgpu::BindGroupLayoutDescriptor chunkBindGroupLayoutDesc = wgpu::BindGroupLayoutDescriptor{};
        chunkBindGroupLayoutDesc.entryCount = chunkBindingLayouts.size();
        chunkBindGroupLayoutDesc.entries = chunkBindingLayouts.data();
        chunkBindGroupLayoutDesc.nextInChain = nullptr;
        m_ChunkBindGroupLayout = m_GraphicsContext.GetDevice().CreateBindGroupLayout(&chunkBindGroupLayoutDesc);

        wgpu::PipelineLayoutDescriptor pipelineLayoutDesc = {};
        pipelineLayoutDesc.nextInChain = nullptr;
        wgpu::BindGroupLayout pipelineLayouts[] = {m_BindGroupLayout, m_ChunkBindGroupLayout};
        pipelineLayoutDesc.bindGroupLayoutCount = 2;
        pipelineLayoutDesc.bindGroupLayouts = pipelineLayouts;
        m_PipelineLayout = m_GraphicsContext.GetDevice().CreatePipelineLayout(&pipelineLayoutDesc);
        pipelineDesc.layout = m_PipelineLayout;

        std::vector<wgpu::BindGroupEntry> bindGroupEntries(3);
        bindGroupEntries[0].binding = 0;
        bindGroupEntries[0].buffer = m_CameraUniformBuffer;
        bindGroupEntries[0].offset = 0;
        bindGroupEntries[0].size = sizeof(Core::Application::CameraUniforms);

        bindGroupEntries[1].binding = 1;
        bindGroupEntries[1].textureView = m_BlockAtlas.GetTextureView();

        bindGroupEntries[2].binding = 2;
        bindGroupEntries[2].sampler = m_BlockAtlas.GetSampler();

        wgpu::BindGroupDescriptor bindGroupDesc = {};
        bindGroupDesc.layout = m_BindGroupLayout;
        bindGroupDesc.entryCount = bindGroupEntries.size();
        bindGroupDesc.entries = bindGroupEntries.data();

        wgpu::BindGroupDescriptor chunkBindGroupDesc = {};
        

        m_BindGroup = m_GraphicsContext.GetDevice().CreateBindGroup(&bindGroupDesc);
        m_RenderPipeline = m_GraphicsContext.GetDevice().CreateRenderPipeline(&pipelineDesc);
        shaderModule = nullptr;

        if (!m_RenderPipeline)
        {
            LOG_ERROR("Failed to create render pipeline!");
        }
    }

    void TerrainRenderer::Render(WorldRenderer *worldRenderer, wgpu::RenderPassEncoder encoder, const glm::vec3 &cameraPosition)
    {
        ZoneScoped;
        encoder.PushDebugGroup("TerrainRenderer::Render");
        encoder.SetPipeline(m_RenderPipeline);
        encoder.SetBindGroup(0, m_BindGroup);
        worldRenderer->Render(encoder, cameraPosition);
        encoder.PopDebugGroup();
    }
}