#include "Logger.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace Minecraft::Core
{
    Ref<spdlog::logger> Log::s_Logger;

    void Log::Init()
    {
        s_Logger = spdlog::stdout_color_mt("Minecraft");
        s_Logger->set_level(spdlog::level::trace);
        s_Logger->set_pattern("[%H:%M:%S] [%n] [%^%l%$] [%t] %v");
    }

    Ref<spdlog::logger>& Log::GetLogger()
    {
        return s_Logger;
    }
}