#pragma once

#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

namespace Vi {
    class Log {
        using Logger = std::shared_ptr<spdlog::logger>;

    public:
        Log() = default;
        ~Log() = default;

        static void init();

        static Logger& getCoreLogger();
        static Logger& getClientLogger();
    private:
        static Logger s_CoreLogger;
        static Logger s_ClientLogger;
    };
}

// Core log macros
#define VI_CORE_TRACE(...)    ::Vi::Log::getCoreLogger()->trace(__VA_ARGS__)
#define VI_CORE_DEBUG(...)    ::Vi::Log::getCoreLogger()->debug(__VA_ARGS__)
#define VI_CORE_INFO(...)     ::Vi::Log::getCoreLogger()->info(__VA_ARGS__)
#define VI_CORE_WARN(...)     ::Vi::Log::getCoreLogger()->warn(__VA_ARGS__)
#define VI_CORE_ERROR(...)    ::Vi::Log::getCoreLogger()->error(__VA_ARGS__)
#define VI_CORE_CRITICAL(...) ::Vi::Log::getCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define VI_APP_TRACE(...)         ::Vi::Log::getClientLogger()->trace(__VA_ARGS__)
#define VI_APP_DEBUG(...)         ::Vi::Log::getClientLogger()->debug(__VA_ARGS__)
#define VI_APP_INFO(...)          ::Vi::Log::getClientLogger()->info(__VA_ARGS__)
#define VI_APP_WARN(...)          ::Vi::Log::getClientLogger()->warn(__VA_ARGS__)
#define VI_APP_ERROR(...)         ::Vi::Log::getClientLogger()->error(__VA_ARGS__)
#define VI_APP_CRITICAL(...)      ::Vi::Log::getClientLogger()->critical(__VA_ARGS__)
