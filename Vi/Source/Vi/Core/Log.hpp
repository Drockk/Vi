#pragma once

// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#pragma warning(pop)

namespace Vi {
	class Log {
	public:
		Log() = default;
		~Log() = default;

		static void init();

		static std::shared_ptr<spdlog::logger>& getCoreLogger() { return s_CoreLogger; }
		static std::shared_ptr<spdlog::logger>& getClientLogger() { return s_ClientLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
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
#define VI_TRACE(...)         ::Vi::Log::getClientLogger()->trace(__VA_ARGS__)
#define VI_DEBUG(...)         ::Vi::Log::getClientLogger()->trace(__VA_ARGS__)
#define VI_INFO(...)          ::Vi::Log::getClientLogger()->info(__VA_ARGS__)
#define VI_WARN(...)          ::Vi::Log::getClientLogger()->warn(__VA_ARGS__)
#define VI_ERROR(...)         ::Vi::Log::getClientLogger()->error(__VA_ARGS__)
#define VI_CRITICAL(...)      ::Vi::Log::getClientLogger()->critical(__VA_ARGS__)
