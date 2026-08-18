#include "Common/Memory.h"

#include <spdlog/spdlog.h>

namespace Minecraft::Core
{
    class Log
    {
    public:
        static void Init();
        static Ref<spdlog::logger>& GetLogger();

    private:
        static Ref<spdlog::logger> s_Logger;
    };
}

#define LOG_TRACE(...) Minecraft::Core::Log::GetLogger()->trace(__VA_ARGS__)
#define LOG_INFO(...) Minecraft::Core::Log::GetLogger()->info(__VA_ARGS__)
#define LOG_WARN(...) Minecraft::Core::Log::GetLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...) Minecraft::Core::Log::GetLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) Minecraft::Core::Log::GetLogger()->critical(__VA_ARGS__)