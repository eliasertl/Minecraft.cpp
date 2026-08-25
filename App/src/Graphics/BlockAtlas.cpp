#include "BlockAtlas.h"
#include "Core/Logger.h"

#include <glm/glm.hpp>
#include <stb_image.h>
#include <algorithm>

namespace Minecraft::Graphics
{
    BlockAtlas::BlockAtlas(GraphicsContext &context, std::vector<Data::BlockType> blockTypes)
        : m_Context(context), m_BlockTypes(std::move(blockTypes))
    {
        ReadTextures();
        CreateAtlasTexture();
    }

    BlockAtlas::~BlockAtlas()
    {
    }

    // Based on https://lisyarus.github.io/blog/posts/texture-packing.html
    void BlockAtlas::CreateAtlasTexture()
    {
        // Generate cpu side texture atlas
        LOG_INFO("Creating block texture atlas");
        if (m_BlockTextureInfos.empty())
            return;

        std::vector<size_t> sorted(m_BlockTextureInfos.size());

        for (size_t i = 0; i < sorted.size(); ++i)
            sorted[i] = i;

        std::sort(sorted.begin(), sorted.end(),
                  [&](size_t a, size_t b)
                  {
                      return m_BlockTextureInfos[a].height >
                             m_BlockTextureInfos[b].height;
                  });

        uint64_t totalArea = 0;
        uint32_t maxSize = 0;

        for (const auto &texture : m_BlockTextureInfos)
        {
            totalArea += static_cast<uint64_t>(texture.width) *
                         static_cast<uint64_t>(texture.height);

            maxSize = std::max(
                maxSize,
                static_cast<uint32_t>(std::max(texture.width, texture.height)));
        }

        uint32_t atlasSize = maxSize;

        while (static_cast<uint64_t>(atlasSize) * atlasSize < totalArea)
            atlasSize *= 2;

        glm::ivec2 atlasSize2(
            static_cast<int>(atlasSize),
            static_cast<int>(atlasSize));

        std::vector<glm::ivec2> ladder;

        glm::ivec2 pen(0, 0);

        struct Region
        {
            glm::ivec2 topLeft;
            glm::ivec2 bottomRight;
        };

        std::vector<Region> regions(m_BlockTextureInfos.size());

        for (size_t index : sorted)
        {
            const BlockTextureInfo &texture = m_BlockTextureInfos[index];

            const int size = texture.width;

            if (size != texture.height)
            {
                LOG_ERROR(
                    "Texture '{}' is not square ({}x{}). "
                    "The current atlas allocator only supports square textures.",
                    texture.id,
                    texture.width,
                    texture.height);

                continue;
            }

            regions[index] =
                {
                    .topLeft = pen,
                    .bottomRight = pen + glm::ivec2(size, size)};

            m_BlockTextureInfos[index].atlasPosition = pen;

            pen.x += size;

            if (!ladder.empty() &&
                ladder.back().y == pen.y + size)
            {
                ladder.back().x = pen.x;
            }
            else
            {
                ladder.push_back(
                    glm::ivec2(
                        pen.x,
                        pen.y + size));
            }

            if (pen.x == atlasSize2.x)
            {
                ladder.pop_back();

                pen.y += size;

                if (!ladder.empty())
                    pen.x = ladder.back().x;
                else
                    pen.x = 0;
            }
        }

        uint32_t usedWidth = 0;
        uint32_t usedHeight = 0;

        for (const Region &region : regions)
        {
            usedWidth = std::max(
                usedWidth,
                static_cast<uint32_t>(region.bottomRight.x));

            usedHeight = std::max(
                usedHeight,
                static_cast<uint32_t>(region.bottomRight.y));
        }

        m_Width = usedWidth;
        m_Height = usedHeight;

        if (m_Width == 0 || m_Height == 0)
            return;

        /*int channels = m_BlockTextureInfos[0].channels;

        for (const auto &texture : m_BlockTextureInfos)
        {
            if (texture.channels != channels)
            {
                LOG_ERROR(
                    "Texture '{}' has {} channels, but atlas expects {}.",
                    texture.id,
                    texture.channels,
                    channels);

                return;
            }
        }*/
        int channels = 4;

        const size_t atlasStride =
            static_cast<size_t>(m_Width) * channels;

        std::vector<uint8_t> data;
        data.resize(
            static_cast<size_t>(m_Width) *
            static_cast<size_t>(m_Height) *
            channels);

        for (size_t i = 0; i < m_BlockTextureInfos.size(); ++i)
        {
            const BlockTextureInfo &texture = m_BlockTextureInfos[i];
            const Region &region = regions[i];

            const int dstX = region.topLeft.x;
            const int dstY = region.topLeft.y;

            const size_t srcStride =
                static_cast<size_t>(texture.width) * channels;

            for (int y = 0; y < texture.height; ++y)
            {
                const uint8_t *src =
                    texture.data +
                    static_cast<size_t>(y) * srcStride;

                uint8_t *dst =
                    data.data() +
                    static_cast<size_t>(dstY + y) * atlasStride +
                    static_cast<size_t>(dstX) * channels;

                std::copy(
                    src,
                    src + srcStride,
                    dst);
            }
        }

        // Create GPU side texture atlas
        wgpu::TextureDescriptor textureDesc{};
        textureDesc.label = "Block Texture Atlas";
        textureDesc.size = {m_Width, m_Height, 1};
        textureDesc.mipLevelCount = 1;
        textureDesc.sampleCount = 1;
        textureDesc.dimension = wgpu::TextureDimension::e2D;
        textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
        textureDesc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;

        m_AtlasTexture = m_Context.GetDevice().CreateTexture(&textureDesc);

        wgpu::TextureViewDescriptor viewDesc{};
        viewDesc.label = "Block Texture Atlas View";
        viewDesc.format = wgpu::TextureFormat::RGBA8Unorm;
        viewDesc.dimension = wgpu::TextureViewDimension::e2D;
        viewDesc.baseMipLevel = 0;
        viewDesc.mipLevelCount = 1;
        viewDesc.baseArrayLayer = 0;
        viewDesc.arrayLayerCount = 1;
        viewDesc.aspect = wgpu::TextureAspect::All;

        m_AtlasTextureView = m_AtlasTexture.CreateView(&viewDesc);

        wgpu::SamplerDescriptor samplerDesc{};
        samplerDesc.magFilter = wgpu::FilterMode::Nearest;
        samplerDesc.minFilter = wgpu::FilterMode::Nearest;
        samplerDesc.mipmapFilter = wgpu::MipmapFilterMode::Nearest;
        samplerDesc.addressModeU = wgpu::AddressMode::ClampToEdge;
        samplerDesc.addressModeV = wgpu::AddressMode::ClampToEdge;

        m_AtlasSampler = m_Context.GetDevice().CreateSampler(&samplerDesc);

        wgpu::TexelCopyTextureInfo destination{};
        destination.texture = m_AtlasTexture;
        destination.mipLevel = 0;
        destination.origin = {0, 0, 0};
        destination.aspect = wgpu::TextureAspect::All;

        wgpu::TexelCopyBufferLayout layout{};
        layout.offset = 0;

        const uint32_t unalignedBytesPerRow =
            m_Width * channels;

        const uint32_t bytesPerRow =
            (unalignedBytesPerRow + 255) & ~255u;

        layout.bytesPerRow = bytesPerRow;
        layout.rowsPerImage = m_Height;

        if (bytesPerRow != unalignedBytesPerRow)
        {
            std::vector<uint8_t> paddedData(
                static_cast<size_t>(bytesPerRow) * m_Height,
                0);

            for (uint32_t y = 0; y < m_Height; ++y)
            {
                std::memcpy(
                    paddedData.data() +
                        static_cast<size_t>(y) * bytesPerRow,

                    data.data() +
                        static_cast<size_t>(y) *
                            unalignedBytesPerRow,

                    unalignedBytesPerRow);
            }

            wgpu::Extent3D writeSize{
                m_Width,
                m_Height,
                1};

            m_Context.GetQueue().WriteTexture(
                &destination,
                paddedData.data(),
                paddedData.size(),
                &layout,
                &writeSize);
        }
        else
        {
            wgpu::Extent3D writeSize{
                m_Width,
                m_Height,
                1};

            m_Context.GetQueue().WriteTexture(
                &destination,
                data.data(),
                data.size(),
                &layout,
                &writeSize);
        }

        LOG_INFO(
            "Created block texture atlas: {}x{} ({} textures)",
            m_Width,
            m_Height,
            m_BlockTextureInfos.size());
    }

    void BlockAtlas::ReadTextures()
    {
        m_BlockTextureInfos.clear();
        m_BlockTextureInfos.reserve(m_BlockTypes.size());

        BlockTextureInfo checkerboardTexture;
        checkerboardTexture.id = "default:missing_texture";
        checkerboardTexture.width = 32;
        checkerboardTexture.height = 32;
        checkerboardTexture.channels = 4;
        std::vector<uint8_t> checkerboardData(checkerboardTexture.width * checkerboardTexture.height * checkerboardTexture.channels);
        for (int y = 0; y < checkerboardTexture.height; ++y)
        {
            for (int x = 0; x < checkerboardTexture.width; ++x)
            {
                int index = (y * checkerboardTexture.width + x) * checkerboardTexture.channels;
                if ((x / 8 + y / 8) % 2 == 0)
                {
                    checkerboardData[index] = 255;     // R
                    checkerboardData[index + 1] = 0;   // G
                    checkerboardData[index + 2] = 255; // B
                    checkerboardData[index + 3] = 255; // A
                }
                else
                {
                    checkerboardData[index] = 0;       // R
                    checkerboardData[index + 1] = 0;   // G
                    checkerboardData[index + 2] = 0;   // B
                    checkerboardData[index + 3] = 255; // A
                }
            }
        }
        checkerboardTexture.data = checkerboardData.data();
        m_BlockTextureInfos.push_back(checkerboardTexture);

        for (auto &blockType : m_BlockTypes)
        {
            BlockTextureInfo textureInfo;
            textureInfo.id = blockType.id;

            int width, height, channels;
            stbi_set_flip_vertically_on_load(true);
            unsigned char *data = stbi_load(blockType.texturePath.c_str(), &width, &height, &channels, 4);
            if (data)
            {
                textureInfo.width = width;
                textureInfo.height = height;
                textureInfo.channels = channels;
                textureInfo.data = data;
                m_BlockTextureInfos.push_back(textureInfo);
            }
            else
            {
                LOG_ERROR("Failed to load texture: {}", blockType.texturePath);
                textureInfo.width = 32;
                textureInfo.height = 32;
                textureInfo.channels = 4;
                textureInfo.data = checkerboardData.data();
                m_BlockTextureInfos.push_back(checkerboardTexture);
            }
        }
    }

    BlockAtlasCoord BlockAtlas::GetBlockTextureCoord(const std::string &blockId) const
    {
        for (const auto &textureInfo : m_BlockTextureInfos)
        {
            if (textureInfo.id == blockId)
            {
                const float x = static_cast<float>(textureInfo.atlasPosition.x);
                const float y = static_cast<float>(textureInfo.atlasPosition.y);
                const float width = static_cast<float>(textureInfo.width);
                const float height = static_cast<float>(textureInfo.height);

                return {
                    {x / static_cast<float>(m_Width),
                     y / static_cast<float>(m_Height)},
                    {(x + width) / static_cast<float>(m_Width),
                     (y + height) / static_cast<float>(m_Height)}};
            }
        }

        auto &errorTexture = m_BlockTextureInfos[0];
        const float x = static_cast<float>(errorTexture.atlasPosition.x);
        const float y = static_cast<float>(errorTexture.atlasPosition.y);
        const float width = static_cast<float>(errorTexture.width);
        const float height = static_cast<float>(errorTexture.height);

        return {
            {x / static_cast<float>(m_Width),
             y / static_cast<float>(m_Height)},
            {(x + width) / static_cast<float>(m_Width),
             (y + height) / static_cast<float>(m_Height)}};
    }

    BlockAtlasCoord BlockAtlas::GetBlockTextureCoord(const Data::BlockType &blockType) const
    {
        return GetBlockTextureCoord(blockType.id);
    }
}