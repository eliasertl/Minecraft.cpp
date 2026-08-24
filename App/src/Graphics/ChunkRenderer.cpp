#include "ChunkRenderer.h"
#include "Core/Logger.h"

namespace Minecraft::Graphics
{
    ChunkRenderer::ChunkRenderer(Data::Chunk &chunk, GraphicsContext &graphicsContext)
        : m_Chunk(chunk), m_GraphicsContext(graphicsContext)
    {
        // Vertex buffer
        struct Vertex
        {
            float position[3];
            float uv[2];
        };

        Vertex vertices[] = {
            // Front  (+Z)
            {{0.5f, 0.5f, 0.5f}, {1.0f, 1.0f}},
            {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f}},
            {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f}},
            {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f}},

            // Back   (-Z)
            {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}},
            {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
            {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
            {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f}},

            // Right  (+X)
            {{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}},
            {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f}},
            {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
            {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f}},

            // Left   (-X)
            {{-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f}},
            {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
            {{-0.5f, -0.5f, 0.5f}, {1.0f, 0.0f}},
            {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f}},

            // Top    (+Y)
            {{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}},
            {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f}},
            {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f}},
            {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f}},

            // Bottom (-Y)
            {{0.5f, -0.5f, 0.5f}, {1.0f, 1.0f}},
            {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
            {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
            {{-0.5f, -0.5f, 0.5f}, {0.0f, 1.0f}},
        };

        m_VertexCount = static_cast<uint32_t>(sizeof(vertices) / sizeof(Vertex));

        wgpu::BufferDescriptor vertexBufferDesc = {};
        vertexBufferDesc.size = sizeof(vertices);
        vertexBufferDesc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
        vertexBufferDesc.mappedAtCreation = false;
        m_VertexBuffer = m_GraphicsContext.GetDevice().CreateBuffer(&vertexBufferDesc);

        m_GraphicsContext.GetQueue().WriteBuffer(m_VertexBuffer, 0, vertices, sizeof(vertices));

        // Index Buffer
        uint16_t indices[6 * 6];
        for (uint16_t face = 0; face < 6; ++face)
        {
            uint16_t base = static_cast<uint16_t>(face * 4);
            uint16_t *f = &indices[face * 6];
            f[0] = base + 0;
            f[1] = base + 1;
            f[2] = base + 2;
            f[3] = base + 0;
            f[4] = base + 3;
            f[5] = base + 1;
        }

        m_IndexCount = static_cast<uint32_t>(sizeof(indices) / sizeof(uint16_t));
        wgpu::BufferDescriptor indexBufferDesc = {};
        indexBufferDesc.size = sizeof(indices);
        indexBufferDesc.usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst;
        indexBufferDesc.mappedAtCreation = false;
        m_IndexBuffer = m_GraphicsContext.GetDevice().CreateBuffer(&indexBufferDesc);

        m_GraphicsContext.GetQueue().WriteBuffer(m_IndexBuffer, 0, indices, sizeof(indices));
        // BuildMesh();
    }

    ChunkRenderer::~ChunkRenderer()
    {

        m_VertexBuffer = nullptr;
        m_IndexBuffer = nullptr;
    }

    void ChunkRenderer::Render(wgpu::RenderPassEncoder encoder)
    {
        encoder.SetVertexBuffer(0, m_VertexBuffer);
        encoder.SetIndexBuffer(m_IndexBuffer, wgpu::IndexFormat::Uint16);
        encoder.DrawIndexed(m_IndexCount, 1, 0, 0, 0);
    }

    void ChunkRenderer::BuildMesh()
    {
        // CURRENTLY NOT IMPLEMENTED
    }
}