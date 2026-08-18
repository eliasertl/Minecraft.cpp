#include "Common/Memory.h"

#include <spdlog/spdlog.h>
#include <format>
#include <webgpu/webgpu_cpp.h>

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

template <>
struct fmt::formatter<wgpu::StringView> : fmt::formatter<std::string_view> {
    template <typename FormatContext>
    auto format(const wgpu::StringView& sv, FormatContext& ctx) const {
        return fmt::formatter<std::string_view>::format(
            static_cast<std::string_view>(sv), ctx);
    }
};

#define LOG_TRACE(...) Minecraft::Core::Log::GetLogger()->trace(__VA_ARGS__)
#define LOG_INFO(...) Minecraft::Core::Log::GetLogger()->info(__VA_ARGS__)
#define LOG_WARN(...) Minecraft::Core::Log::GetLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...) Minecraft::Core::Log::GetLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) Minecraft::Core::Log::GetLogger()->critical(__VA_ARGS__)