#pragma once

#include "BlockAtlas.h"

namespace Minecraft::Graphics {
    class BlockModel {
        public:
            BlockModel() = default;
            virtual ~BlockModel() = default;

            virtual BlockAtlasID getSideTextureId() const { return 0; }
            virtual BlockAtlasID getTopTextureId() const { return 0; };
            virtual BlockAtlasID getBottomTextureId() const { return 0; };
    };

    class CubeBlockModel : public BlockModel {
        public:
            CubeBlockModel(BlockAtlasID sideTextureId, BlockAtlasID topTextureId, BlockAtlasID bottomTextureId)
                : m_SideTextureId(sideTextureId), m_TopTextureId(topTextureId), m_BottomTextureId(bottomTextureId) {}

            virtual BlockAtlasID getSideTextureId() const override { return m_SideTextureId; }
            virtual BlockAtlasID getTopTextureId() const override { return m_TopTextureId; }
            virtual BlockAtlasID getBottomTextureId() const override { return m_BottomTextureId; }

        private:
            BlockAtlasID m_SideTextureId;
            BlockAtlasID m_TopTextureId;
            BlockAtlasID m_BottomTextureId;
    };

    class CrossBlockModel : public BlockModel {
        public:
            CrossBlockModel(BlockAtlasID textureId)
                : m_TextureId(textureId) {}

            virtual BlockAtlasID getSideTextureId() const override { return m_TextureId; }
            virtual BlockAtlasID getTopTextureId() const override { return m_TextureId; }
            virtual BlockAtlasID getBottomTextureId() const override { return m_TextureId; }

        private:
            BlockAtlasID m_TextureId;
    };
}