#pragma once

#include <string>

namespace Minecraft {
    namespace Common {
        std::string ReadFileToString(const std::string& filepath);
    }

    using namespace Common;
}