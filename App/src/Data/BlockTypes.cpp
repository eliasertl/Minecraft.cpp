#include "BlockTypes.h"
#include "Core/Logger.h"

#include <exception>
#include <tracy/Tracy.hpp>

namespace Minecraft::Data
{
    std::vector<BlockType> BlockTypes::s_BlockTypes;
    bool BlockTypes::s_isRegistrationFinished = false;

    BlockType BlockTypes::getBlockTypeById(const std::string& id)
    {
        for (const auto& blockType : s_BlockTypes)
        {
            if (blockType.id == id)
            {
                return blockType;
            }
        }
        return BlockType{};
    }

    BlockType BlockTypes::getBlockTypeByName(const std::string& name)
    {
        for (const auto& blockType : s_BlockTypes)
        {
            if (blockType.name == name)
            {
                return blockType;
            }
        }
        return BlockType{};
    }

    void BlockTypes::registerBlockType(const BlockType& blockType)
    {
        ZoneScoped;
        if(s_isRegistrationFinished)
        {
            LOG_ERROR("Cannot register block types after registration is finished.");
            throw std::runtime_error("Cannot register block types after registration is finished.");
        }
        s_BlockTypes.push_back(blockType);
    }

    void BlockTypes::finishRegistration()
    {
        s_isRegistrationFinished = true;
    }
}