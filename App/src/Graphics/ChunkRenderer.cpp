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

        // uint16_t Buffer
        uint32_t indexBufferSize = (m_IndexCount > 0) ? m_IndexCount * sizeof(uint16_t) : 128 * sizeof(uint16_t);
        wgpu::BufferDescriptor indexBufferDesc = {};
        indexBufferDesc.size = indexBufferSize;
        indexBufferDesc.usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst;
        indexBufferDesc.mappedAtCreation = false;
        m_IndexBuffer = m_GraphicsContext.GetDevice().CreateBuffer(&indexBufferDesc);
        m_AllocatedIndexCount = indexBufferSize / sizeof(uint16_t);

        LOG_INFO("Initialized buffers: VertexCount = {}, AllocatedVertexCount = {}, IndexCount = {}, AllocatedIndexCount = {}", m_VertexCount, m_AllocatedVertexCount, m_IndexCount, m_AllocatedIndexCount);
    }

    ChunkRenderer::~ChunkRenderer()
    {

        m_VertexBuffer = nullptr;
        m_IndexBuffer = nullptr;
    }

    void ChunkRenderer::Render(wgpu::RenderPassEncoder encoder)
    {
        if(m_Chunk.isDirty())
        {
            BuildMesh();
            m_Chunk.markClean();
        }
        encoder.SetVertexBuffer(0, m_VertexBuffer);
        encoder.SetIndexBuffer(m_IndexBuffer, wgpu::IndexFormat::Uint16);
        encoder.DrawIndexed(m_IndexCount, 1, 0, 0, 0);
    }

    void ChunkRenderer::BuildMesh()
    {
        std::vector<Vertex> vertices;
        std::vector<uint16_t> indices;

        vertices.reserve(Data::CHUNK_LENGTH * Data::CHUNK_LENGTH * Data::CHUNK_LENGTH * 24);
        indices.reserve(Data::CHUNK_LENGTH * Data::CHUNK_LENGTH * Data::CHUNK_LENGTH * 36);

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

                    const uint16_t baseIndex = vertices.size();

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
                        const uint16_t faceBase = baseIndex + face * 4;

                        indices.push_back(faceBase + 0);
                        indices.push_back(faceBase + 1);
                        indices.push_back(faceBase + 2);

                        indices.push_back(faceBase + 0);
                        indices.push_back(faceBase + 3);
                        indices.push_back(faceBase + 1);
                    }
                }
            }
        }

        m_VertexCount = vertices.size();
        m_IndexCount = indices.size();

        if (m_VertexCount > m_AllocatedVertexCount || m_IndexCount > m_AllocatedIndexCount)
        {
            LOG_INFO("Reallocating buffers: VertexCount = {}, AllocatedVertexCount = {}, IndexCount = {}, AllocatedIndexCount = {}", m_VertexCount, m_AllocatedVertexCount, m_IndexCount, m_AllocatedIndexCount);
            InitBuffers();
        }

        m_GraphicsContext.GetQueue().WriteBuffer(m_VertexBuffer, 0, vertices.data(), m_VertexCount * sizeof(Vertex));
        m_GraphicsContext.GetQueue().WriteBuffer(m_IndexBuffer, 0, indices.data(), m_IndexCount * sizeof(uint16_t));
    }
}