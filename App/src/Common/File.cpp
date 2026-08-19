#include "File.h"
#include "Core/Logger.h"

#include <fstream>

std::string Minecraft::Common::ReadFileToString(const std::string &filepath)
{
    std::ifstream file(filepath, std::ios::in | std::ios::binary);
    if (!file)
    {
        LOG_ERROR("Failed to open file: {}", filepath);
        return "";
    }

    std::string content;
    file.seekg(0, std::ios::end);
    content.resize(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(&content[0], content.size());
    file.close();

    return content;
}