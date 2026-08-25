#pragma once

#include <string>
#include <vector>

#include "Graphics/BlockAtlas.h"

namespace Minecraft::Data
{
    struct BlockType
    {
        std::string name = "Unknown";
        std::string id = "default:unknown";
        Graphics::BlockAtlasID textureId = 0;
    };

    class BlockTypes
    {
    public:
        static BlockType getBlockTypeById(const std::string& id);
        static BlockType getBlockTypeByName(const std::string& name);

        static void registerBlockType(const BlockType& blockType);
        static const std::vector<BlockType>& getRegisteredBlockTypes() { return s_BlockTypes; }

        static void finishRegistration();
        
    private:
        static bool s_isRegistrationFinished;
        static std::vector<BlockType> s_BlockTypes;

        friend class Application;
    };
};