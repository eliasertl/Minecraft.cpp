#pragma once

#include "Data/BlockTypes.h"
#include "GraphicsContext.h"
#include <webgpu/webgpu_cpp.h>
#include <glm/glm.hpp>

namespace Minecraft::Graphics
{
    struct BlockAtlasCoord
    {
        glm::vec2 uvMin;
        glm::vec2 uvMax;
    };

    class BlockAtlas
    {
    public:
        BlockAtlas(GraphicsContext& context, std::vector<Data::BlockType> blockTypes);
        ~BlockAtlas();

        BlockAtlasCoord GetBlockTextureCoord(const std::string& blockId) const;
        BlockAtlasCoord GetBlockTextureCoord(const Data::BlockType& blockType) const;

        inline wgpu::Texture GetTexture() const { return m_AtlasTexture; }
        inline wgpu::TextureView GetTextureView() const { return m_AtlasTextureView; }
        inline wgpu::Sampler GetSampler() const { return m_AtlasSampler; }
        
        inline uint32_t GetWidth() const { return m_Width; }
        inline uint32_t GetHeight() const { return m_Height; }

    private:
        struct BlockTextureInfo
        {
            std::string id;
            uint32_t width, height;
            glm::ivec2 atlasPosition{0, 0};
            uint8_t channels;
            uint8_t* data = nullptr;
        };

        void ReadTextures();
        void CreateAtlasTexture();

    private:
        GraphicsContext& m_Context;
        wgpu::Texture m_AtlasTexture;
        wgpu::TextureView m_AtlasTextureView;
        wgpu::Sampler m_AtlasSampler;
        std::vector<Data::BlockType> m_BlockTypes;
        std::vector<BlockTextureInfo> m_BlockTextureInfos;
        uint32_t m_Width = 0, m_Height = 0;
    };
};