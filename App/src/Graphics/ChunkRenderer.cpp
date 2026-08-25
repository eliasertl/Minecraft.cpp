#include "ChunkRenderer.h"
#include "Core/Logger.h"

namespace Minecraft::Graphics
{
    struct Vertex
    {
        float position[3];
        float uv[2];
    };

    ChunkRenderer::ChunkRenderer(Data::Chunk &chunk, GraphicsContext &graphicsContext)
        : m_Chunk(chunk), m_GraphicsContext(graphicsContext)
    {
        InitBuffers();
        BuildMesh();
    }

    void ChunkRenderer::InitBuffers()
    {
        // Vertex Buffer
        uint32_t vertexBufferSize = (m_VertexCount > 0) ? m_VertexCount * sizeof(Vertex) : 128 * sizeof(Vertex);
        wgpu::BufferDescriptor vertexBufferDesc = {};
        vertexBufferDesc.size = vertexBufferSize;
        vertexBufferDesc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
        vertexBufferDesc.mappedAtCreation = false;
        m_VertexBuffer = m_GraphicsContext.GetDevice().CreateBuffer(&vertexBufferDesc);
        m_AllocatedVertexCount = vertexBufferSize / sizeof(Vertex);

        // Index Buffer
        uint32_t indexBufferSize = (m_IndexCount > 0) ? m_IndexCount * sizeof(uint32_t) : 128 * sizeof(uint32_t);
        wgpu::BufferDescriptor indexBufferDesc = {};
        indexBufferDesc.size = indexBufferSize;
        indexBufferDesc.usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst;
        indexBufferDesc.mappedAtCreation = false;
        m_IndexBuffer = m_GraphicsContext.GetDevice().CreateBuffer(&indexBufferDesc);
        m_AllocatedIndexCount = indexBufferSize / sizeof(uint32_t);

#ifdef MC_DEBUG
        // Wireframe Index Buffer
        uint32_t wireframeIndexBufferSize = (m_WireframeIndexCount > 0) ? m_WireframeIndexCount * sizeof(uint32_t) : 128 * sizeof(uint32_t);
        wgpu::BufferDescriptor wireframeIndexBufferDesc = {};
        wireframeIndexBufferDesc.size = wireframeIndexBufferSize;
        wireframeIndexBufferDesc.usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst;
        wireframeIndexBufferDesc.mappedAtCreation = false;
        m_WireframeIndexBuffer = m_GraphicsContext.GetDevice().CreateBuffer(&wireframeIndexBufferDesc);
        m_AllocatedWireframeIndexCount = wireframeIndexBufferSize / sizeof(uint32_t);
#endif

#ifndef MC_DEBUG
        LOG_INFO("Initialized buffers: VertexCount = {}, AllocatedVertexCount = {}, IndexCount = {}, AllocatedIndexCount = {}", m_VertexCount, m_AllocatedVertexCount, m_IndexCount, m_AllocatedIndexCount);
#else
        LOG_INFO("Initialized buffers: VertexCount = {}, AllocatedVertexCount = {}, IndexCount = {}, AllocatedIndexCount = {}, WireframeIndexCount = {}, AllocatedWireframeIndexCount = {}", m_VertexCount, m_AllocatedVertexCount, m_IndexCount, m_AllocatedIndexCount, m_WireframeIndexCount, m_AllocatedWireframeIndexCount);
#endif
    }

    ChunkRenderer::~ChunkRenderer()
    {
        m_VertexBuffer = nullptr;
        m_IndexBuffer = nullptr;
#ifdef MC_DEBUG
        m_WireframeIndexBuffer = nullptr;
#endif
    }

    void ChunkRenderer::Render(wgpu::RenderPassEncoder encoder)
    {
        encoder.PushDebugGroup("ChunkRenderer::Render");
        if (m_Chunk.isDirty())
        {
            BuildMesh();
            m_Chunk.markClean();
        }
        encoder.SetVertexBuffer(0, m_VertexBuffer);
        encoder.SetIndexBuffer(m_IndexBuffer, wgpu::IndexFormat::Uint32);
        encoder.DrawIndexed(m_IndexCount, 1, 0, 0, 0);
        encoder.PopDebugGroup();
    }

    void ChunkRenderer::RenderWireframe(wgpu::RenderPassEncoder encoder)
    {
        encoder.PushDebugGroup("ChunkRenderer::RenderWireframe");
        if (m_Chunk.isDirty())
        {
            BuildMesh();
            m_Chunk.markClean();
        }
        encoder.SetVertexBuffer(0, m_VertexBuffer);
        encoder.SetIndexBuffer(m_WireframeIndexBuffer, wgpu::IndexFormat::Uint32);
        encoder.DrawIndexed(m_WireframeIndexCount, 1, 0, 0, 0);
        encoder.PopDebugGroup();
    }

    void ChunkRenderer::BuildMesh()
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<uint32_t> wireFrameIndices;

        vertices.reserve(Data::CHUNK_LENGTH * Data::CHUNK_LENGTH * Data::CHUNK_LENGTH * 24);
        indices.reserve(Data::CHUNK_LENGTH * Data::CHUNK_LENGTH * Data::CHUNK_LENGTH * 36);
        wireFrameIndices.reserve(Data::CHUNK_LENGTH * Data::CHUNK_LENGTH * Data::CHUNK_LENGTH * 60);

        for (uint32_t x = 0; x < Data::CHUNK_LENGTH; ++x)
        {
            for (uint32_t y = 0; y < Data::CHUNK_LENGTH; ++y)
            {
                for (uint32_t z = 0; z < Data::CHUNK_LENGTH; ++z)
                {
                    Data::Block *block = m_Chunk.getBlock(x, y, z);
                    if (!block || block->id == 0)
                        continue;

                    const float minX = static_cast<float>(x);
                    const float minY = static_cast<float>(y);
                    const float minZ = static_cast<float>(z);

                    const float maxX = minX + 1.0f;
                    const float maxY = minY + 1.0f;
                    const float maxZ = minZ + 1.0f;

                    const uint32_t baseIndex = static_cast<uint32_t>(vertices.size());

                    // Front (+Z)
                    vertices.push_back({{maxX, maxY, maxZ}, {1.0f, 1.0f}});
                    vertices.push_back({{minX, minY, maxZ}, {0.0f, 0.0f}});
                    vertices.push_back({{maxX, minY, maxZ}, {1.0f, 0.0f}});
                    vertices.push_back({{minX, maxY, maxZ}, {0.0f, 1.0f}});

                    // Back (-Z)
                    vertices.push_back({{minX, maxY, minZ}, {1.0f, 1.0f}});
                    vertices.push_back({{maxX, minY, minZ}, {0.0f, 0.0f}});
                    vertices.push_back({{minX, minY, minZ}, {1.0f, 0.0f}});
                    vertices.push_back({{maxX, maxY, minZ}, {0.0f, 1.0f}});

                    // Right (+X)
                    vertices.push_back({{maxX, maxY, minZ}, {1.0f, 1.0f}});
                    vertices.push_back({{maxX, minY, maxZ}, {0.0f, 0.0f}});
                    vertices.push_back({{maxX, minY, minZ}, {1.0f, 0.0f}});
                    vertices.push_back({{maxX, maxY, maxZ}, {0.0f, 1.0f}});

                    // Left (-X)
                    vertices.push_back({{minX, maxY, maxZ}, {1.0f, 1.0f}});
                    vertices.push_back({{minX, minY, minZ}, {0.0f, 0.0f}});
                    vertices.push_back({{minX, minY, maxZ}, {1.0f, 0.0f}});
                    vertices.push_back({{minX, maxY, minZ}, {0.0f, 1.0f}});

                    // Top (+Y)
                    vertices.push_back({{maxX, maxY, minZ}, {1.0f, 1.0f}});
                    vertices.push_back({{minX, maxY, maxZ}, {0.0f, 0.0f}});
                    vertices.push_back({{maxX, maxY, maxZ}, {1.0f, 0.0f}});
                    vertices.push_back({{minX, maxY, minZ}, {0.0f, 1.0f}});

                    // Bottom (-Y)
                    vertices.push_back({{maxX, minY, maxZ}, {1.0f, 1.0f}});
                    vertices.push_back({{minX, minY, minZ}, {0.0f, 0.0f}});
                    vertices.push_back({{maxX, minY, minZ}, {1.0f, 0.0f}});
                    vertices.push_back({{minX, minY, maxZ}, {0.0f, 1.0f}});

                    for (uint32_t face = 0; face < 6; ++face)
                    {
                        const uint32_t faceBase = baseIndex + face * 4;

                        indices.push_back(faceBase + 0);
                        indices.push_back(faceBase + 1);
                        indices.push_back(faceBase + 2);

                        indices.push_back(faceBase + 0);
                        indices.push_back(faceBase + 3);
                        indices.push_back(faceBase + 1);

#ifdef MC_DEBUG
                        wireFrameIndices.push_back(faceBase + 0);
                        wireFrameIndices.push_back(faceBase + 1);
                        wireFrameIndices.push_back(faceBase + 1);
                        wireFrameIndices.push_back(faceBase + 2);
                        wireFrameIndices.push_back(faceBase + 2);
                        wireFrameIndices.push_back(faceBase + 0);
                        wireFrameIndices.push_back(faceBase + 0);
                        wireFrameIndices.push_back(faceBase + 3);
                        wireFrameIndices.push_back(faceBase + 3);
                        wireFrameIndices.push_back(faceBase + 1);
#endif
                    }
                }
            }
        }

        m_VertexCount = static_cast<uint32_t>(vertices.size());
        m_IndexCount = static_cast<uint32_t>(indices.size());
#ifdef MC_DEBUG
        m_WireframeIndexCount = static_cast<uint32_t>(wireFrameIndices.size());
#endif

        if (m_VertexCount > m_AllocatedVertexCount || m_IndexCount > m_AllocatedIndexCount
#ifdef MC_DEBUG
            || m_WireframeIndexCount > m_AllocatedWireframeIndexCount
#endif
        )
        {
            LOG_INFO("Reallocating buffers: VertexCount = {}, AllocatedVertexCount = {}, IndexCount = {}, AllocatedIndexCount = {}", m_VertexCount, m_AllocatedVertexCount, m_IndexCount, m_AllocatedIndexCount);
            InitBuffers();
        }

        m_GraphicsContext.GetQueue().WriteBuffer(m_VertexBuffer, 0, vertices.data(), m_VertexCount * sizeof(Vertex));
        m_GraphicsContext.GetQueue().WriteBuffer(m_IndexBuffer, 0, indices.data(), m_IndexCount * sizeof(uint32_t));
#ifdef MC_DEBUG
        m_GraphicsContext.GetQueue().WriteBuffer(m_WireframeIndexBuffer, 0, wireFrameIndices.data(), m_WireframeIndexCount * sizeof(uint32_t));
#endif
    }
}