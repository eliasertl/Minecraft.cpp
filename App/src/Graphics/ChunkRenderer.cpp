#include "ChunkRenderer.h"
#include "Data/BlockTypes.h"
#include "Core/Logger.h"

#include <tracy/Tracy.hpp>

namespace Minecraft::Graphics
{
    ChunkRenderer::ChunkRenderer(Data::Chunk &chunk, GraphicsContext &graphicsContext, const BlockAtlas &blockAtlas)
        : m_Chunk(chunk), m_GraphicsContext(graphicsContext), m_BlockAtlas(blockAtlas)
    {
        InitBuffers();
        BuildMesh();
    }

    void ChunkRenderer::InitBuffers()
    {
        // Vertex Buffer
        uint32_t vertexBufferSize = (m_VertexCount > 0) ? m_VertexCount * sizeof(TerrainVertex) : 128 * sizeof(TerrainVertex);
        wgpu::BufferDescriptor vertexBufferDesc = {};
        vertexBufferDesc.label = "[ChunkRenderer] Vertex Buffer";
        vertexBufferDesc.size = vertexBufferSize;
        vertexBufferDesc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
        vertexBufferDesc.mappedAtCreation = false;
        m_VertexBuffer = m_GraphicsContext.GetDevice().CreateBuffer(&vertexBufferDesc);
        m_AllocatedVertexCount = vertexBufferSize / sizeof(TerrainVertex);

        // Index Buffer
        uint32_t indexBufferSize = (m_IndexCount > 0) ? m_IndexCount * sizeof(uint32_t) : 128 * sizeof(uint32_t);
        wgpu::BufferDescriptor indexBufferDesc = {};
        indexBufferDesc.label = "[ChunkRenderer] Index Buffer";
        indexBufferDesc.size = indexBufferSize;
        indexBufferDesc.usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst;
        indexBufferDesc.mappedAtCreation = false;
        m_IndexBuffer = m_GraphicsContext.GetDevice().CreateBuffer(&indexBufferDesc);
        m_AllocatedIndexCount = indexBufferSize / sizeof(uint32_t);

        // Wireframe Vertex Buffer
        uint32_t wireframeVertexBufferSize = (m_WireframeVertexCount > 0) ? m_WireframeVertexCount * sizeof(WireframeVertex) : 128 * sizeof(WireframeVertex);
        wgpu::BufferDescriptor wireframeVertexBufferDesc = {};
        wireframeVertexBufferDesc.label = "[ChunkRenderer] Wireframe Vertex Buffer";
        wireframeVertexBufferDesc.size = wireframeVertexBufferSize;
        wireframeVertexBufferDesc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
        wireframeVertexBufferDesc.mappedAtCreation = false;
        m_WireframeVertexBuffer = m_GraphicsContext.GetDevice().CreateBuffer(&wireframeVertexBufferDesc);
        m_AllocatedWireframeVertexCount = wireframeVertexBufferSize / sizeof(WireframeVertex);

        // Wireframe Index Buffer
        uint32_t wireframeIndexBufferSize = (m_WireframeIndexCount > 0) ? m_WireframeIndexCount * sizeof(uint32_t) : 128 * sizeof(uint32_t);
        wgpu::BufferDescriptor wireframeIndexBufferDesc = {};
        wireframeIndexBufferDesc.label = "[ChunkRenderer] Wireframe Index Buffer";
        wireframeIndexBufferDesc.size = wireframeIndexBufferSize;
        wireframeIndexBufferDesc.usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst;
        wireframeIndexBufferDesc.mappedAtCreation = false;
        m_WireframeIndexBuffer = m_GraphicsContext.GetDevice().CreateBuffer(&wireframeIndexBufferDesc);
        m_AllocatedWireframeIndexCount = wireframeIndexBufferSize / sizeof(uint32_t);
    }

    ChunkRenderer::~ChunkRenderer()
    {
        m_VertexBuffer = nullptr;
        m_IndexBuffer = nullptr;

        m_WireframeIndexBuffer = nullptr;
    }

    void ChunkRenderer::Render(wgpu::RenderPassEncoder encoder)
    {
        ZoneScoped;
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
        ZoneScoped;
        encoder.PushDebugGroup("ChunkRenderer::RenderWireframe");
        if (m_Chunk.isDirty())
        {
            BuildMesh();
            m_Chunk.markClean();
        }
        encoder.SetVertexBuffer(0, m_WireframeVertexBuffer);
        encoder.SetIndexBuffer(m_WireframeIndexBuffer, wgpu::IndexFormat::Uint32);
        encoder.DrawIndexed(m_WireframeIndexCount, 1, 0, 0, 0);
        encoder.PopDebugGroup();
    }

    void ChunkRenderer::BuildMesh()
    {
        ZoneScoped;
        std::vector<TerrainVertex> vertices;
        std::vector<uint32_t> indices;       
        std::vector<WireframeVertex> wireframeVertices;
        std::vector<uint32_t> wireframeIndices;

        vertices.reserve(Data::CHUNK_LENGTH * Data::CHUNK_WIDTH * Data::CHUNK_HEIGHT * 24);
        indices.reserve(Data::CHUNK_LENGTH * Data::CHUNK_WIDTH * Data::CHUNK_HEIGHT * 36);
        wireframeVertices.reserve(Data::CHUNK_LENGTH * Data::CHUNK_WIDTH * Data::CHUNK_HEIGHT * 24);
        wireframeIndices.reserve(Data::CHUNK_LENGTH * Data::CHUNK_WIDTH * Data::CHUNK_HEIGHT * 60);

        for (uint32_t x = 0; x < Data::CHUNK_LENGTH; ++x)
        {
            for (uint32_t y = 0; y < Data::CHUNK_HEIGHT; ++y)
            {
                for (uint32_t z = 0; z < Data::CHUNK_WIDTH; ++z)
                {
                    Data::Block *block = m_Chunk.getBlock(x, y, z);
                    if (!block || block->id == 0)
                        continue;

                    ActiveFaces activeFaces;
                    if (x == 0 || !m_Chunk.getBlock(x - 1, y, z) || m_Chunk.getBlock(x - 1, y, z)->id == 0)
                        activeFaces.left = true;
                    if (x == Data::CHUNK_LENGTH - 1 || !m_Chunk.getBlock(x + 1, y, z) || m_Chunk.getBlock(x + 1, y, z)->id == 0)
                        activeFaces.right = true;
                    if (y == 0 || !m_Chunk.getBlock(x, y - 1, z) || m_Chunk.getBlock(x, y - 1, z)->id == 0)
                        activeFaces.bottom = true;
                    if (y == Data::CHUNK_HEIGHT - 1 || !m_Chunk.getBlock(x, y + 1, z) || m_Chunk.getBlock(x, y + 1, z)->id == 0)
                        activeFaces.top = true;
                    if (z == 0 || !m_Chunk.getBlock(x, y, z - 1) || m_Chunk.getBlock(x, y, z - 1)->id == 0)
                        activeFaces.back = true;
                    if (z == Data::CHUNK_WIDTH - 1 || !m_Chunk.getBlock(x, y, z + 1) || m_Chunk.getBlock(x, y, z + 1)->id == 0)
                        activeFaces.front = true;

                    // minus one because AIR=0
                    const Data::BlockType &blockType = Data::BlockTypes::getRegisteredBlockTypes()[block->id - 1];
                    BlockAtlasCoord atlasCoord = m_BlockAtlas.GetBlockTextureCoord(blockType.textureId);

                    CreateCube(vertices, indices, wireframeVertices, wireframeIndices, x, y, z, atlasCoord, activeFaces);
                }
            }
        }

        m_VertexCount = static_cast<uint32_t>(vertices.size());
        m_IndexCount = static_cast<uint32_t>(indices.size());
        m_WireframeVertexCount = static_cast<uint32_t>(wireframeVertices.size());
        m_WireframeIndexCount = static_cast<uint32_t>(wireframeIndices.size());

        if (m_VertexCount > m_AllocatedVertexCount || m_IndexCount > m_AllocatedIndexCount
            || m_WireframeIndexCount > m_AllocatedWireframeIndexCount || m_WireframeVertexCount > m_AllocatedWireframeVertexCount
        )
        {
            LOG_INFO("Reallocating buffers: VertexCount = {}, AllocatedVertexCount = {}, IndexCount = {}, AllocatedIndexCount = {}", m_VertexCount, m_AllocatedVertexCount, m_IndexCount, m_AllocatedIndexCount);
            InitBuffers();
        }

        m_GraphicsContext.GetQueue().WriteBuffer(m_VertexBuffer, 0, vertices.data(), m_VertexCount * sizeof(TerrainVertex));
        m_GraphicsContext.GetQueue().WriteBuffer(m_IndexBuffer, 0, indices.data(), m_IndexCount * sizeof(uint32_t));
        m_GraphicsContext.GetQueue().WriteBuffer(m_WireframeVertexBuffer, 0, wireframeVertices.data(), m_WireframeVertexCount * sizeof(WireframeVertex));   
        m_GraphicsContext.GetQueue().WriteBuffer(m_WireframeIndexBuffer, 0, wireframeIndices.data(), m_WireframeIndexCount * sizeof(uint32_t));
    }

    void ChunkRenderer::CreateCube(std::vector<TerrainVertex> &vertices,
                                   std::vector<uint32_t> &indices,
                                   std::vector<WireframeVertex> &wireframeVertices,
                                   std::vector<uint32_t> &wireframeIndices,
                                   uint32_t x, uint32_t y, uint32_t z,
                                   BlockAtlasCoord atlasCoord,
                                   ActiveFaces activeFaces)
    {
        const float minX = static_cast<float>(x);
        const float minY = static_cast<float>(y);
        const float minZ = static_cast<float>(z);

        const float maxX = minX + 1.0f;
        const float maxY = minY + 1.0f;
        const float maxZ = minZ + 1.0f;

        const glm::vec3 color(1.0f, 1.0f, 1.0f);

        // Front (+Z)
        if (activeFaces.front)
        {
            vertices.push_back({{maxX, maxY, maxZ}, {0, 0, 1}, atlasCoord.uvMax});
            vertices.push_back({{minX, minY, maxZ}, {0, 0, 1}, atlasCoord.uvMin});
            vertices.push_back({{maxX, minY, maxZ}, {0, 0, 1}, {atlasCoord.uvMax.x, atlasCoord.uvMin.y}});
            vertices.push_back({{minX, maxY, maxZ}, {0, 0, 1}, {atlasCoord.uvMin.x, atlasCoord.uvMax.y}});

            wireframeVertices.push_back({{maxX, maxY, maxZ}, color});
            wireframeVertices.push_back({{minX, minY, maxZ}, color});
            wireframeVertices.push_back({{maxX, minY, maxZ}, color});
            wireframeVertices.push_back({{minX, maxY, maxZ}, color});

            AppendQuadIndices(indices, wireframeIndices, static_cast<uint32_t>(vertices.size()) - 4);
        }

        // Back (-Z)
        if (activeFaces.back)
        {
            vertices.push_back({{minX, maxY, minZ}, {0, 0, -1}, atlasCoord.uvMax});
            vertices.push_back({{maxX, minY, minZ}, {0, 0, -1}, atlasCoord.uvMin});
            vertices.push_back({{minX, minY, minZ}, {0, 0, -1}, {atlasCoord.uvMax.x, atlasCoord.uvMin.y}});
            vertices.push_back({{maxX, maxY, minZ}, {0, 0, -1}, {atlasCoord.uvMin.x, atlasCoord.uvMax.y}});
            
            wireframeVertices.push_back({{minX, maxY, minZ}, color});
            wireframeVertices.push_back({{maxX, minY, minZ}, color});
            wireframeVertices.push_back({{minX, minY, minZ}, color});
            wireframeVertices.push_back({{maxX, maxY, minZ}, color});

            AppendQuadIndices(indices, wireframeIndices, static_cast<uint32_t>(vertices.size()) - 4);
        }

        // Right (+X)
        if (activeFaces.right)
        {
            vertices.push_back({{maxX, maxY, maxZ}, {1, 0, 0}, atlasCoord.uvMax});
            vertices.push_back({{maxX, minY, minZ}, {1, 0, 0}, atlasCoord.uvMin});
            vertices.push_back({{maxX, minY, maxZ}, {1, 0, 0}, {atlasCoord.uvMax.x, atlasCoord.uvMin.y}});
            vertices.push_back({{maxX, maxY, minZ}, {1, 0, 0}, {atlasCoord.uvMin.x, atlasCoord.uvMax.y}});

            wireframeVertices.push_back({{maxX, maxY, maxZ}, color});
            wireframeVertices.push_back({{maxX, minY, minZ}, color});
            wireframeVertices.push_back({{maxX, minY, maxZ}, color});
            wireframeVertices.push_back({{maxX, maxY, minZ}, color});

            AppendQuadIndices(indices, wireframeIndices, static_cast<uint32_t>(vertices.size()) - 4, true);
        }

        // Left (-X)
        if (activeFaces.left)
        {
            vertices.push_back({{minX, maxY, maxZ}, {-1, 0, 0}, atlasCoord.uvMax});
            vertices.push_back({{minX, minY, minZ}, {-1, 0, 0}, atlasCoord.uvMin});
            vertices.push_back({{minX, minY, maxZ}, {-1, 0, 0}, {atlasCoord.uvMax.x, atlasCoord.uvMin.y}});
            vertices.push_back({{minX, maxY, minZ}, {-1, 0, 0}, {atlasCoord.uvMin.x, atlasCoord.uvMax.y}});

            wireframeVertices.push_back({{minX, maxY, maxZ}, color});
            wireframeVertices.push_back({{minX, minY, minZ}, color});
            wireframeVertices.push_back({{minX, minY, maxZ}, color});
            wireframeVertices.push_back({{minX, maxY, minZ}, color});

            AppendQuadIndices(indices, wireframeIndices, static_cast<uint32_t>(vertices.size()) - 4);
        }

        // Top (+Y)
        if (activeFaces.top)
        {
            vertices.push_back({{maxX, maxY, minZ}, {0, 1, 0}, atlasCoord.uvMax});
            vertices.push_back({{minX, maxY, maxZ}, {0, 1, 0}, atlasCoord.uvMin});
            vertices.push_back({{maxX, maxY, maxZ}, {0, 1, 0}, {atlasCoord.uvMax.x, atlasCoord.uvMin.y}});
            vertices.push_back({{minX, maxY, minZ}, {0, 1, 0}, {atlasCoord.uvMin.x, atlasCoord.uvMax.y}});

            wireframeVertices.push_back({{maxX, maxY, minZ}, color});
            wireframeVertices.push_back({{minX, maxY, maxZ}, color});
            wireframeVertices.push_back({{maxX, maxY, maxZ}, color});
            wireframeVertices.push_back({{minX, maxY, minZ}, color});

            AppendQuadIndices(indices, wireframeIndices, static_cast<uint32_t>(vertices.size()) - 4);
        }

        // Bottom (-Y)
        if (activeFaces.bottom)
        {
            vertices.push_back({{maxX, minY, maxZ}, {0, -1, 0}, atlasCoord.uvMax});
            vertices.push_back({{minX, minY, minZ}, {0, -1, 0}, atlasCoord.uvMin});
            vertices.push_back({{maxX, minY, minZ}, {0, -1, 0}, {atlasCoord.uvMax.x, atlasCoord.uvMin.y}});
            vertices.push_back({{minX, minY, maxZ}, {0, -1, 0}, {atlasCoord.uvMin.x, atlasCoord.uvMax.y}});

            wireframeVertices.push_back({{maxX, minY, maxZ}, color});
            wireframeVertices.push_back({{minX, minY, minZ}, color});
            wireframeVertices.push_back({{maxX, minY, minZ}, color});
            wireframeVertices.push_back({{minX, minY, maxZ}, color});

            AppendQuadIndices(indices, wireframeIndices, static_cast<uint32_t>(vertices.size()) - 4);
        }
    }

    void ChunkRenderer::AppendQuadIndices(std::vector<uint32_t> &indices,
                                          std::vector<uint32_t> &wireframeIndices,
                                          uint32_t faceBase,
                                          bool invertWindingOrder)
    {
        if (invertWindingOrder)
        {
            indices.push_back(faceBase + 0);
            indices.push_back(faceBase + 2);
            indices.push_back(faceBase + 1);

            indices.push_back(faceBase + 0);
            indices.push_back(faceBase + 1);
            indices.push_back(faceBase + 3);

            wireframeIndices.push_back(faceBase + 0);
            wireframeIndices.push_back(faceBase + 2);
            wireframeIndices.push_back(faceBase + 2);
            wireframeIndices.push_back(faceBase + 1);
            wireframeIndices.push_back(faceBase + 1);
            wireframeIndices.push_back(faceBase + 0);
            wireframeIndices.push_back(faceBase + 0);
            wireframeIndices.push_back(faceBase + 1);
            wireframeIndices.push_back(faceBase + 1);
            wireframeIndices.push_back(faceBase + 3);
            wireframeIndices.push_back(faceBase + 3);
            wireframeIndices.push_back(faceBase + 0);
        }
        else
        {
            indices.push_back(faceBase + 0);
            indices.push_back(faceBase + 1);
            indices.push_back(faceBase + 2);

            indices.push_back(faceBase + 0);
            indices.push_back(faceBase + 3);
            indices.push_back(faceBase + 1);

            wireframeIndices.push_back(faceBase + 0);
            wireframeIndices.push_back(faceBase + 1);
            wireframeIndices.push_back(faceBase + 1);
            wireframeIndices.push_back(faceBase + 2);
            wireframeIndices.push_back(faceBase + 2);
            wireframeIndices.push_back(faceBase + 0);
            wireframeIndices.push_back(faceBase + 0);
            wireframeIndices.push_back(faceBase + 3);
            wireframeIndices.push_back(faceBase + 3);
            wireframeIndices.push_back(faceBase + 1);
        }
    }
}