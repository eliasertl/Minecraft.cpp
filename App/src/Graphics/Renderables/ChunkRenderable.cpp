#include "ChunkRenderable.h"
#include "Data/BlockTypes.h"
#include "Data/World.h"
#include "Core/Logger.h"

#include <tracy/Tracy.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Minecraft::Graphics
{
    ChunkRenderable::ChunkRenderable(Data::Chunk &chunk, GraphicsContext &graphicsContext, const BlockAtlas &blockAtlas)
        : m_Chunk(chunk), m_GraphicsContext(graphicsContext), m_BlockAtlas(blockAtlas)
    {
        InitBuffers();
        BuildMesh();
    }

    void ChunkRenderable::InitBindGroup()
    {
        std::vector<wgpu::BindGroupLayoutEntry> chunkBindingLayouts(1);
        chunkBindingLayouts[0].binding = 0;
        chunkBindingLayouts[0].visibility = wgpu::ShaderStage::Vertex;
        chunkBindingLayouts[0].buffer.type = wgpu::BufferBindingType::Uniform;
        chunkBindingLayouts[0].buffer.minBindingSize = sizeof(ChunkUniforms);

        wgpu::BindGroupLayoutDescriptor chunkBindGroupLayoutDesc = {};
        chunkBindGroupLayoutDesc.entryCount = chunkBindingLayouts.size();
        chunkBindGroupLayoutDesc.entries = chunkBindingLayouts.data();
        chunkBindGroupLayoutDesc.nextInChain = nullptr;
        m_ChunkBindGroupLayout = m_GraphicsContext.GetDevice().CreateBindGroupLayout(&chunkBindGroupLayoutDesc);

        std::vector<wgpu::BindGroupEntry> bindGroupEntries(1);
        bindGroupEntries[0].binding = 0;
        bindGroupEntries[0].buffer = m_UniformBuffer;
        bindGroupEntries[0].offset = 0;
        bindGroupEntries[0].size = sizeof(ChunkUniforms);

        wgpu::BindGroupDescriptor bindGroupDesc = {};
        bindGroupDesc.layout = m_ChunkBindGroupLayout;
        bindGroupDesc.entryCount = bindGroupEntries.size();
        bindGroupDesc.entries = bindGroupEntries.data();
        m_ChunkBindGroup = m_GraphicsContext.GetDevice().CreateBindGroup(&bindGroupDesc);
    }

    void ChunkRenderable::InitBuffers()
    {
        // Vertex Buffer
        uint32_t vertexBufferSize = (m_VertexCount > 0) ? m_VertexCount * sizeof(TerrainVertex) : 128 * sizeof(TerrainVertex);
        wgpu::BufferDescriptor vertexBufferDesc = {};
        vertexBufferDesc.label = "[ChunkRenderable] Vertex Buffer";
        vertexBufferDesc.size = vertexBufferSize;
        vertexBufferDesc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
        vertexBufferDesc.mappedAtCreation = false;
        m_VertexBuffer = m_GraphicsContext.GetDevice().CreateBuffer(&vertexBufferDesc);
        m_AllocatedVertexCount = vertexBufferSize / sizeof(TerrainVertex);

        // Index Buffer
        uint32_t indexBufferSize = (m_IndexCount > 0) ? m_IndexCount * sizeof(uint32_t) : 128 * sizeof(uint32_t);
        wgpu::BufferDescriptor indexBufferDesc = {};
        indexBufferDesc.label = "[ChunkRenderable] Index Buffer";
        indexBufferDesc.size = indexBufferSize;
        indexBufferDesc.usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst;
        indexBufferDesc.mappedAtCreation = false;
        m_IndexBuffer = m_GraphicsContext.GetDevice().CreateBuffer(&indexBufferDesc);
        m_AllocatedIndexCount = indexBufferSize / sizeof(uint32_t);

        // Wireframe Vertex Buffer
        uint32_t wireframeVertexBufferSize = (m_WireframeVertexCount > 0) ? m_WireframeVertexCount * sizeof(WireframeVertex) : 128 * sizeof(WireframeVertex);
        wgpu::BufferDescriptor wireframeVertexBufferDesc = {};
        wireframeVertexBufferDesc.label = "[ChunkRenderable] Wireframe Vertex Buffer";
        wireframeVertexBufferDesc.size = wireframeVertexBufferSize;
        wireframeVertexBufferDesc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
        wireframeVertexBufferDesc.mappedAtCreation = false;
        m_WireframeVertexBuffer = m_GraphicsContext.GetDevice().CreateBuffer(&wireframeVertexBufferDesc);
        m_AllocatedWireframeVertexCount = wireframeVertexBufferSize / sizeof(WireframeVertex);

        // Wireframe Index Buffer
        uint32_t wireframeIndexBufferSize = (m_WireframeIndexCount > 0) ? m_WireframeIndexCount * sizeof(uint32_t) : 128 * sizeof(uint32_t);
        wgpu::BufferDescriptor wireframeIndexBufferDesc = {};
        wireframeIndexBufferDesc.label = "[ChunkRenderable] Wireframe Index Buffer";
        wireframeIndexBufferDesc.size = wireframeIndexBufferSize;
        wireframeIndexBufferDesc.usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst;
        wireframeIndexBufferDesc.mappedAtCreation = false;
        m_WireframeIndexBuffer = m_GraphicsContext.GetDevice().CreateBuffer(&wireframeIndexBufferDesc);
        m_AllocatedWireframeIndexCount = wireframeIndexBufferSize / sizeof(uint32_t);

        // Uniform Buffer
        wgpu::BufferDescriptor bufferDesc = {};
        bufferDesc.size = sizeof(ChunkUniforms);
        bufferDesc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
        bufferDesc.mappedAtCreation = false;
        m_UniformBuffer = m_GraphicsContext.GetDevice().CreateBuffer(&bufferDesc);

        InitBindGroup();
    }

    ChunkRenderable::~ChunkRenderable()
    {
        m_ChunkBindGroup = nullptr;
        m_ChunkBindGroupLayout = nullptr;
        m_VertexBuffer.Destroy();
        m_IndexBuffer.Destroy();
        m_WireframeVertexBuffer.Destroy();
        m_WireframeIndexBuffer.Destroy();
        m_UniformBuffer.Destroy();
    }

    void ChunkRenderable::Render(wgpu::RenderPassEncoder& encoder, glm::vec3 cameraPosition)
    {
        ZoneScoped;
        encoder.PushDebugGroup("ChunkRenderable::Render");
        if (m_Chunk.isDirty())
        {
            BuildMesh();
            m_Chunk.markClean();
        }
        encoder.SetBindGroup(1, m_ChunkBindGroup);
        encoder.SetVertexBuffer(0, m_VertexBuffer);
        encoder.SetIndexBuffer(m_IndexBuffer, wgpu::IndexFormat::Uint32);
        encoder.DrawIndexed(m_IndexCount, 1, 0, 0, 0);
        encoder.PopDebugGroup();
    }

    void ChunkRenderable::RenderWireframe(wgpu::RenderPassEncoder& encoder, glm::vec3 cameraPosition)
    {
        ZoneScoped;
        encoder.PushDebugGroup("ChunkRenderable::RenderWireframe");
        if (m_Chunk.isDirty())
        {
            BuildMesh();
            m_Chunk.markClean();
        }
        encoder.SetBindGroup(1, m_ChunkBindGroup);
        encoder.SetVertexBuffer(0, m_WireframeVertexBuffer);
        encoder.SetIndexBuffer(m_WireframeIndexBuffer, wgpu::IndexFormat::Uint32);
        encoder.DrawIndexed(m_WireframeIndexCount, 1, 0, 0, 0);
        encoder.PopDebugGroup();
    }

    void ChunkRenderable::BuildMesh()
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

                    int32_t xOffset = static_cast<int32_t>(x) + m_Chunk.getChunkPosition().x * Data::CHUNK_LENGTH;
                    int32_t yOffset = static_cast<int32_t>(y);
                    int32_t zOffset = static_cast<int32_t>(z) + m_Chunk.getChunkPosition().y * Data::CHUNK_WIDTH;

                    auto world = m_Chunk.getWorld();

                    ActiveFaces activeFaces;
                    {
                        ZoneScopedN("Determine Active Faces");
                        const Data::Block *leftNeighbor = world->getBlock(xOffset - 1, yOffset, zOffset);
                        const Data::Block *rightNeighbor = world->getBlock(xOffset + 1, yOffset, zOffset);
                        const Data::Block *bottomNeighbor = world->getBlock(xOffset, yOffset - 1, zOffset);
                        const Data::Block *topNeighbor = world->getBlock(xOffset, yOffset + 1, zOffset);
                        const Data::Block *backNeighbor = world->getBlock(xOffset, yOffset, zOffset - 1);
                        const Data::Block *frontNeighbor = world->getBlock(xOffset, yOffset, zOffset + 1);

                        activeFaces.left = leftNeighbor == nullptr || leftNeighbor->id == 0;
                        activeFaces.right = rightNeighbor == nullptr || rightNeighbor->id == 0;
                        activeFaces.bottom = bottomNeighbor == nullptr || bottomNeighbor->id == 0;
                        activeFaces.top = topNeighbor == nullptr || topNeighbor->id == 0;
                        activeFaces.back = backNeighbor == nullptr || backNeighbor->id == 0;
                        activeFaces.front = frontNeighbor == nullptr || frontNeighbor->id == 0;
                    }

                    // minus one because AIR=0
                    const Data::BlockType &blockType = Data::BlockTypes::getRegisteredBlockTypes()[block->id - 1];
                    CubeUVs atlasCoord;
                    BlockAtlasCoord topCoords = m_BlockAtlas.GetBlockTextureCoord(blockType.model->getTopTextureId());
                    BlockAtlasCoord bottomCoords = m_BlockAtlas.GetBlockTextureCoord(blockType.model->getBottomTextureId());
                    BlockAtlasCoord sideCoords = m_BlockAtlas.GetBlockTextureCoord(blockType.model->getSideTextureId());

                    atlasCoord.topMax = topCoords.uvMax;
                    atlasCoord.topMin = topCoords.uvMin;
                    atlasCoord.bottomMax = bottomCoords.uvMax;
                    atlasCoord.bottomMin = bottomCoords.uvMin;
                    atlasCoord.sideMax = sideCoords.uvMax;
                    atlasCoord.sideMin = sideCoords.uvMin;

                    CreateCube(vertices, indices, wireframeVertices, wireframeIndices, x, y, z, atlasCoord, activeFaces);
                }
            }
        }

        m_VertexCount = static_cast<uint32_t>(vertices.size());
        m_IndexCount = static_cast<uint32_t>(indices.size());
        m_WireframeVertexCount = static_cast<uint32_t>(wireframeVertices.size());
        m_WireframeIndexCount = static_cast<uint32_t>(wireframeIndices.size());

        if (m_VertexCount > m_AllocatedVertexCount || m_IndexCount > m_AllocatedIndexCount || m_WireframeIndexCount > m_AllocatedWireframeIndexCount || m_WireframeVertexCount > m_AllocatedWireframeVertexCount)
        {
            InitBuffers();
        }

        ChunkUniforms chunkUniforms{};
        const glm::ivec2 chunkPosition = m_Chunk.getChunkPosition();
        chunkUniforms.transform = glm::translate(glm::mat4(1.0f), glm::vec3(
                                                                      static_cast<float>(chunkPosition.x) * Data::CHUNK_LENGTH,
                                                                      0.0f,
                                                                      static_cast<float>(chunkPosition.y) * Data::CHUNK_WIDTH));

        m_GraphicsContext.GetQueue().WriteBuffer(m_VertexBuffer, 0, vertices.data(), m_VertexCount * sizeof(TerrainVertex));
        m_GraphicsContext.GetQueue().WriteBuffer(m_IndexBuffer, 0, indices.data(), m_IndexCount * sizeof(uint32_t));
        m_GraphicsContext.GetQueue().WriteBuffer(m_WireframeVertexBuffer, 0, wireframeVertices.data(), m_WireframeVertexCount * sizeof(WireframeVertex));
        m_GraphicsContext.GetQueue().WriteBuffer(m_WireframeIndexBuffer, 0, wireframeIndices.data(), m_WireframeIndexCount * sizeof(uint32_t));
        m_GraphicsContext.GetQueue().WriteBuffer(m_UniformBuffer, 0, &chunkUniforms, sizeof(ChunkUniforms));
    }

    void ChunkRenderable::CreateCube(std::vector<TerrainVertex> &vertices,
                                   std::vector<uint32_t> &indices,
                                   std::vector<WireframeVertex> &wireframeVertices,
                                   std::vector<uint32_t> &wireframeIndices,
                                   uint32_t x, uint32_t y, uint32_t z,
                                   CubeUVs uvs,
                                   ActiveFaces activeFaces)
    {
        ZoneScoped;
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
            vertices.push_back({{maxX, maxY, maxZ}, {0, 0, 1}, uvs.sideMax});
            vertices.push_back({{minX, minY, maxZ}, {0, 0, 1}, uvs.sideMin});
            vertices.push_back({{maxX, minY, maxZ}, {0, 0, 1}, {uvs.sideMax.x, uvs.sideMin.y}});
            vertices.push_back({{minX, maxY, maxZ}, {0, 0, 1}, {uvs.sideMin.x, uvs.sideMax.y}});

            wireframeVertices.push_back({{maxX, maxY, maxZ}, color});
            wireframeVertices.push_back({{minX, minY, maxZ}, color});
            wireframeVertices.push_back({{maxX, minY, maxZ}, color});
            wireframeVertices.push_back({{minX, maxY, maxZ}, color});

            AppendQuadIndices(indices, wireframeIndices, static_cast<uint32_t>(vertices.size()) - 4);
        }

        // Back (-Z)
        if (activeFaces.back)
        {
            vertices.push_back({{minX, maxY, minZ}, {0, 0, -1}, uvs.sideMax});
            vertices.push_back({{maxX, minY, minZ}, {0, 0, -1}, uvs.sideMin});
            vertices.push_back({{minX, minY, minZ}, {0, 0, -1}, {uvs.sideMax.x, uvs.sideMin.y}});
            vertices.push_back({{maxX, maxY, minZ}, {0, 0, -1}, {uvs.sideMin.x, uvs.sideMax.y}});

            wireframeVertices.push_back({{minX, maxY, minZ}, color});
            wireframeVertices.push_back({{maxX, minY, minZ}, color});
            wireframeVertices.push_back({{minX, minY, minZ}, color});
            wireframeVertices.push_back({{maxX, maxY, minZ}, color});

            AppendQuadIndices(indices, wireframeIndices, static_cast<uint32_t>(vertices.size()) - 4);
        }

        // Right (+X)
        if (activeFaces.right)
        {
            vertices.push_back({{maxX, maxY, maxZ}, {1, 0, 0}, uvs.sideMax});
            vertices.push_back({{maxX, minY, minZ}, {1, 0, 0}, uvs.sideMin});
            vertices.push_back({{maxX, minY, maxZ}, {1, 0, 0}, {uvs.sideMax.x, uvs.sideMin.y}});
            vertices.push_back({{maxX, maxY, minZ}, {1, 0, 0}, {uvs.sideMin.x, uvs.sideMax.y}});

            wireframeVertices.push_back({{maxX, maxY, maxZ}, color});
            wireframeVertices.push_back({{maxX, minY, minZ}, color});
            wireframeVertices.push_back({{maxX, minY, maxZ}, color});
            wireframeVertices.push_back({{maxX, maxY, minZ}, color});

            AppendQuadIndices(indices, wireframeIndices, static_cast<uint32_t>(vertices.size()) - 4, true);
        }

        // Left (-X)
        if (activeFaces.left)
        {
            vertices.push_back({{minX, maxY, maxZ}, {-1, 0, 0}, uvs.sideMax});
            vertices.push_back({{minX, minY, minZ}, {-1, 0, 0}, uvs.sideMin});
            vertices.push_back({{minX, minY, maxZ}, {-1, 0, 0}, {uvs.sideMax.x, uvs.sideMin.y}});
            vertices.push_back({{minX, maxY, minZ}, {-1, 0, 0}, {uvs.sideMin.x, uvs.sideMax.y}});

            wireframeVertices.push_back({{minX, maxY, maxZ}, color});
            wireframeVertices.push_back({{minX, minY, minZ}, color});
            wireframeVertices.push_back({{minX, minY, maxZ}, color});
            wireframeVertices.push_back({{minX, maxY, minZ}, color});

            AppendQuadIndices(indices, wireframeIndices, static_cast<uint32_t>(vertices.size()) - 4);
        }

        // Top (+Y)
        if (activeFaces.top)
        {
            vertices.push_back({{maxX, maxY, minZ}, {0, 1, 0}, uvs.topMax});
            vertices.push_back({{minX, maxY, maxZ}, {0, 1, 0}, uvs.topMin});
            vertices.push_back({{maxX, maxY, maxZ}, {0, 1, 0}, {uvs.topMax.x, uvs.topMin.y}});
            vertices.push_back({{minX, maxY, minZ}, {0, 1, 0}, {uvs.topMin.x, uvs.topMax.y}});

            wireframeVertices.push_back({{maxX, maxY, minZ}, color});
            wireframeVertices.push_back({{minX, maxY, maxZ}, color});
            wireframeVertices.push_back({{maxX, maxY, maxZ}, color});
            wireframeVertices.push_back({{minX, maxY, minZ}, color});

            AppendQuadIndices(indices, wireframeIndices, static_cast<uint32_t>(vertices.size()) - 4);
        }

        // Bottom (-Y)
        if (activeFaces.bottom)
        {
            vertices.push_back({{maxX, minY, maxZ}, {0, -1, 0}, uvs.bottomMax});
            vertices.push_back({{minX, minY, minZ}, {0, -1, 0}, uvs.bottomMin});
            vertices.push_back({{maxX, minY, minZ}, {0, -1, 0}, {uvs.bottomMax.x, uvs.bottomMin.y}});
            vertices.push_back({{minX, minY, maxZ}, {0, -1, 0}, {uvs.bottomMin.x, uvs.bottomMax.y}});

            wireframeVertices.push_back({{maxX, minY, maxZ}, color});
            wireframeVertices.push_back({{minX, minY, minZ}, color});
            wireframeVertices.push_back({{maxX, minY, minZ}, color});
            wireframeVertices.push_back({{minX, minY, maxZ}, color});

            AppendQuadIndices(indices, wireframeIndices, static_cast<uint32_t>(vertices.size()) - 4);
        }
    }

    void ChunkRenderable::AppendQuadIndices(std::vector<uint32_t> &indices,
                                          std::vector<uint32_t> &wireframeIndices,
                                          uint32_t faceBase,
                                          bool invertWindingOrder)
    {
        ZoneScoped;
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