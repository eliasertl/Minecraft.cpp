#include "Application.h"
#include "Logger.h"
#include "Common/Input.h"
#include "Graphics/GraphicsProfiling.h"

#include <stdio.h>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>
#include <random>
#include <imgui.h>
#include <tracy/Tracy.hpp>

namespace Minecraft::Core
{
    struct ApplicationSettings
    {
        bool renderWireframe = false;
    };

    const int TestChunkGenerationStartPoint = -3;
    const int TestChunkGenerationEndPoint = 3;

    static ApplicationSettings s_ApplicationSettings;
    Application *Application::s_Instance = nullptr;

    Application::Application()
    {
        ZoneScoped;
        s_Instance = this;
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
        glm::mat4 cameraTransform = glm::mat4(1.0f);
        cameraTransform = glm::translate(cameraTransform, glm::vec3(Data::CHUNK_LENGTH * 0.5f, Data::CHUNK_HEIGHT + 10, Data::CHUNK_WIDTH * 1.5f));
        cameraTransform = glm::rotate(cameraTransform, glm::radians(-30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        m_Camera->SetTransform(cameraTransform);
        m_Camera->SetFOV(70.0f);
        m_ImGuiContext = CreateScope<Graphics::ImGuiContext>(*m_Window, m_GraphicsContext->GetDevice(), m_GraphicsContext->GetQueue(), m_GraphicsContext->GetSurfaceFormat(), m_GraphicsContext->GetDepthFormat());
        Input::Init(m_Window.get());

        m_BlockAtlas = CreateScope<Graphics::BlockAtlas>(*m_GraphicsContext);
        Graphics::BlockAtlasID cobblestoneTextureId = m_BlockAtlas->RegisterBlockTexture("assets/textures/cobblestone.png");
        Graphics::BlockAtlasID dirtTextureId = m_BlockAtlas->RegisterBlockTexture("assets/textures/dirt.png");
        Graphics::BlockAtlasID planksTextureId = m_BlockAtlas->RegisterBlockTexture("assets/textures/planks.png");
        Graphics::BlockAtlasID stoneTextureId = m_BlockAtlas->RegisterBlockTexture("assets/textures/stone.png");
        Graphics::BlockAtlasID grassTopTextureId = m_BlockAtlas->RegisterBlockTexture("assets/textures/grass_top.png");
        Graphics::BlockAtlasID grassSideTextureId = m_BlockAtlas->RegisterBlockTexture("assets/textures/grass_side.png");

        auto cobblestoneModel = CreateRef<Graphics::CrossBlockModel>(cobblestoneTextureId);
        auto dirtModel = CreateRef<Graphics::CrossBlockModel>(dirtTextureId);
        auto planksModel = CreateRef<Graphics::CrossBlockModel>(planksTextureId);
        auto stoneModel = CreateRef<Graphics::CrossBlockModel>(stoneTextureId);
        auto grassModel = CreateRef<Graphics::CubeBlockModel>(grassSideTextureId, grassTopTextureId, dirtTextureId);

        Data::BlockTypes::registerBlockType({"Cobblestone", "default:cobblestone", cobblestoneModel}); // 1
        Data::BlockTypes::registerBlockType({"Dirt", "default:dirt", dirtModel}); // 2
        Data::BlockTypes::registerBlockType({"Planks", "default:planks", planksModel}); // 3
        Data::BlockTypes::registerBlockType({"Stone", "default:stone", stoneModel}); // 4
        Data::BlockTypes::registerBlockType({"Grass", "default:grass", grassModel}); // 5
        Data::BlockTypes::finishRegistration();
        m_BlockAtlas->Finalize();

        wgpu::BufferDescriptor cameraUniformBufferDesc = {};
        cameraUniformBufferDesc.size = sizeof(CameraUniforms);
        cameraUniformBufferDesc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
        cameraUniformBufferDesc.mappedAtCreation = false;
        m_CameraUniformBuffer = m_GraphicsContext->GetDevice().CreateBuffer(&cameraUniformBufferDesc);

        m_World = CreateScope<Data::World>();
        for (int i = TestChunkGenerationStartPoint; i < TestChunkGenerationEndPoint; i++)
        {
            for (int j = TestChunkGenerationStartPoint; j < TestChunkGenerationEndPoint; j++)
            {
                m_World->addEmptyChunk({i, j});
            }
        }
        SetTestChunksRandom();

        m_WorldRenderable = CreateScope<Graphics::WorldRenderable>(*m_World, *m_GraphicsContext, *m_BlockAtlas);
        
        m_TerrainRenderer = CreateScope<Graphics::TerrainRenderer>(*m_GraphicsContext, m_CameraUniformBuffer, *m_BlockAtlas);
        m_WireframeRenderer = CreateScope<Graphics::WireframeRenderer>(*m_GraphicsContext, m_CameraUniformBuffer);

        FrameMark;
    }

    void Application::SetTestChunksRandom()
    {
        ZoneScoped;
        auto gen = std::bind(std::uniform_int_distribution<>(0, 4), std::default_random_engine());
        auto topgen = std::bind(std::uniform_int_distribution<>(0, 5), std::default_random_engine());
        for (int i = TestChunkGenerationStartPoint; i < TestChunkGenerationEndPoint; i++)
        {
            for (int j = TestChunkGenerationStartPoint; j < TestChunkGenerationEndPoint; j++)
            {
                Data::Chunk *chunk = m_World->getChunk({i, j});

                for (uint16_t x = 0; x < Data::CHUNK_LENGTH; x++)
                {
                    for (uint16_t y = 0; y < Data::CHUNK_HEIGHT; y++)
                    {
                        for (uint16_t z = 0; z < Data::CHUNK_WIDTH; z++)
                        {
                            if(y == Data::CHUNK_HEIGHT - 1)
                                chunk->setBlock(x, y, z, topgen(), 0);
                            else
                                chunk->setBlock(x, y, z, gen(), 0);
                        }
                    }
                }
            }
        }
    }

    void Application::SetTestChunksFlat()
    {
        ZoneScoped;
        for (int i = TestChunkGenerationStartPoint; i < TestChunkGenerationEndPoint; i++)
        {
            for (int j = TestChunkGenerationStartPoint; j < TestChunkGenerationEndPoint; j++)
            {
                Data::Chunk *chunk = m_World->getChunk({i, j});

                for (uint16_t x = 0; x < Data::CHUNK_LENGTH; x++)
                {
                    for (uint16_t y = 0; y < Data::CHUNK_HEIGHT; y++)
                    {
                        for (uint16_t z = 0; z < Data::CHUNK_WIDTH; z++)
                        {
                            if (y == Data::CHUNK_HEIGHT - 1)
                                chunk->setBlock(x, y, z, 1, 0);
                            else
                                chunk->setBlock(x, y, z, 0, 0);
                        }
                    }
                }
            }
        }
    }

    void Application::SetTestChunksEmpty()
    {
        ZoneScoped;
        for (int i = TestChunkGenerationStartPoint; i < TestChunkGenerationEndPoint; i++)
        {
            for (int j = TestChunkGenerationStartPoint; j < TestChunkGenerationEndPoint; j++)
            {
                Data::Chunk *chunk = m_World->getChunk({i, j});

                for (uint16_t x = 0; x < Data::CHUNK_LENGTH; x++)
                {
                    for (uint16_t y = 0; y < Data::CHUNK_HEIGHT; y++)
                    {
                        for (uint16_t z = 0; z < Data::CHUNK_WIDTH; z++)
                        {
                            chunk->setBlock(x, y, z, 0, 0);
                        }
                    }
                }
            }
        }
    }

    void Application::SetTestChunksFull()
    {
        ZoneScoped;
        for (int i = TestChunkGenerationStartPoint; i < TestChunkGenerationEndPoint; i++)
        {
            for (int j = TestChunkGenerationStartPoint; j < TestChunkGenerationEndPoint; j++)
            {
                Data::Chunk *chunk = m_World->getChunk({i, j});

                for (uint16_t x = 0; x < Data::CHUNK_LENGTH; x++)
                {
                    for (uint16_t y = 0; y < Data::CHUNK_HEIGHT; y++)
                    {
                        for (uint16_t z = 0; z < Data::CHUNK_WIDTH; z++)
                        {
                            if (y == Data::CHUNK_HEIGHT - 1)
                                chunk->setBlock(x, y, z, 5, 0); // Grass
                            else if (y >= Data::CHUNK_HEIGHT - 6)
                                chunk->setBlock(x, y, z, 2, 0); // Dirt
                            else
                                chunk->setBlock(x, y, z, 4, 0); // Stone
                        }
                    }
                }
            }
        }
    }

    Application::~Application()
    {
        ZoneScoped;
        LOG_INFO("Shutting down Application");
        m_BlockAtlas.release();
        m_World.release();
        m_ImGuiContext.release();
        m_GraphicsContext.release();
        m_Window.release();
    }

    void Application::OnFrameStart()
    {
        ZoneScoped;

        float deltaTime = m_Window->GetDeltaTime();

        constexpr float smoothing = 0.01f;
        m_SmoothedDeltaTime += (deltaTime - m_SmoothedDeltaTime) * smoothing;

        Input::Update();
        m_Window->Update();
        m_Camera->Update(deltaTime);
        m_ImGuiContext->NewFrame();
    }

    void Application::OnUI(float deltaTime)
    {
        ZoneScoped;
        ImGui::Begin("Info");
        ImGui::Text("FOV: %.2f", glm::degrees(m_Camera->GetFOV()));
        ImGui::Text("Width: %.2f, Height: %.2f", m_Camera->GetViewSize().x, m_Camera->GetViewSize().y);
        ImGui::Text("Position: (%.2f, %.2f, %.2f)", m_Camera->GetTransform()[3][0], m_Camera->GetTransform()[3][1], m_Camera->GetTransform()[3][2]);

        ImGui::SeparatorText("Stats");
        ImGui::Text("FPS: %.2f", 1.0f / m_SmoothedDeltaTime);
        ImGui::Text("Frame Time: %.2f ms", m_SmoothedDeltaTime * 1000.0f);
        ImGui::Text("Window Size: %d x %d", m_Window->GetWidth(), m_Window->GetHeight());

        ImGui::SeparatorText("Block Atlas");
        ImGui::Text("Registered Block Types: %d", Data::BlockTypes::getRegisteredBlockTypes().size());
        ImGui::Text("Atlas Size: %dx%d", m_BlockAtlas->GetWidth(), m_BlockAtlas->GetHeight());
        glm::vec2 aspectAdjustedSize = glm::vec2(256.0f, 256.0f * (static_cast<float>(m_BlockAtlas->GetHeight()) / static_cast<float>(m_BlockAtlas->GetWidth())));
        ImGui::Image(reinterpret_cast<ImTextureID>(m_BlockAtlas->GetTextureView().Get()), {aspectAdjustedSize.x, aspectAdjustedSize.y}, {0, 1}, {1, 0});

        ImGui::SeparatorText("Controls");
        ImGui::Checkbox("Render Wireframe", &s_ApplicationSettings.renderWireframe);

        if (ImGui::Button("Random Chunk"))
        {
            SetTestChunksRandom();
        }
        if (ImGui::Button("Flat Chunk"))
        {
            SetTestChunksFlat();
        }
        if (ImGui::Button("Empty Chunk"))
        {
            SetTestChunksEmpty();
        }
        if (ImGui::Button("Full Chunk"))
        {
            SetTestChunksFull();
        }
        ImGui::End();
    }

    void Application::OnUpdate(float deltaTime)
    {
        ZoneScoped;
        auto [texture, view] = m_GraphicsContext->AcquireNextTexture();

        wgpu::RenderPassDescriptor renderPassDesc = {};
        wgpu::RenderPassColorAttachment renderPassColorAttachment = {};
        wgpu::RenderPassDepthStencilAttachment depthStencilAttachment = {};
        {
            ZoneScopedN("Create Render Pass Descriptor");
            renderPassDesc.nextInChain = nullptr;

            renderPassColorAttachment.view = view;
            renderPassColorAttachment.resolveTarget = nullptr;
            renderPassColorAttachment.loadOp = wgpu::LoadOp::Clear;
            renderPassColorAttachment.storeOp = wgpu::StoreOp::Store;
            renderPassColorAttachment.clearValue = wgpu::Color{0.0, 0.0, 0.0, 1.0};
            renderPassColorAttachment.depthSlice = wgpu::kDepthSliceUndefined;

            renderPassDesc.colorAttachmentCount = 1;
            renderPassDesc.colorAttachments = &renderPassColorAttachment;
            renderPassDesc.depthStencilAttachment = nullptr;
            renderPassDesc.timestampWrites = nullptr;
            renderPassDesc.label = "Terrain Render Pass";

            depthStencilAttachment.view = m_GraphicsContext->GetDepthTextureView();
            depthStencilAttachment.depthLoadOp = wgpu::LoadOp::Clear;
            depthStencilAttachment.depthStoreOp = wgpu::StoreOp::Store;
            depthStencilAttachment.depthClearValue = 1.0f;

            renderPassDesc.depthStencilAttachment = &depthStencilAttachment;
        }

        wgpu::CommandEncoder encoder;
        {
            ZoneScopedN("Create Command Encoder");
            encoder = m_GraphicsContext->GetDevice().CreateCommandEncoder();
        }
        {
            ZoneScopedN("Update Camera Uniform Buffer");
            CameraUniforms cameraUniforms;
            cameraUniforms.viewProjection = m_Camera->GetViewProjection();
            m_GraphicsContext->GetQueue().WriteBuffer(m_CameraUniformBuffer, 0, &cameraUniforms, sizeof(CameraUniforms));
        }
        {
            TracyGPUZoneN(encoder, renderPassDesc, "Render Pass");
            wgpu::RenderPassEncoder renderPassEncoder = encoder.BeginRenderPass(&renderPassDesc);

            m_TerrainRenderer->Render(m_WorldRenderable.get(), renderPassEncoder, m_Camera->GetPosition());
            if (s_ApplicationSettings.renderWireframe)
            {
                m_WireframeRenderer->Render(m_WorldRenderable.get(), renderPassEncoder, m_Camera->GetPosition());
            }

            m_ImGuiContext->Render(renderPassEncoder);

            renderPassEncoder.End();
        }
        {
            ZoneScopedN("Submit Commands");
            wgpu::CommandBuffer commands = encoder.Finish();
            m_GraphicsContext->GetQueue().Submit(1, &commands);
        }
        {
            ZoneScopedN("Present");
            view = nullptr;
            m_GraphicsContext->GetSurface().Present();
            m_GraphicsContext->GetDevice().Tick();
            m_GraphicsContext->FrameEnd();
        }
    }

    void Application::OnFrameEnd()
    {
    }

    void Application::Run()
    {
        LOG_INFO("Running Application");
        while (!m_Window->ShouldClose())
        {
            ZoneScopedN("Application::Run Loop");
            OnFrameStart();
            OnUI(m_Window->GetDeltaTime());
            OnUpdate(m_Window->GetDeltaTime());
            OnFrameEnd();
            FrameMark;
        }
    }
}