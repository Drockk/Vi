#pragma once

#include "Vi/Core/Base.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"

// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

namespace Vi {
	class Log {
	public:
		static void init();

		static Ref<spdlog::logger>& getCoreLogger() { return s_CoreLogger; }
		static Ref<spdlog::logger>& getClientLogger() { return s_ClientLogger; }
	private:
		static Ref<spdlog::logger> s_CoreLogger;
		static Ref<spdlog::logger> s_ClientLogger;
	};

}

template<typename OStream, glm::length_t L, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::vec<L, T, Q>& vector) {
	return os << glm::to_string(vector);
}

template<typename OStream, glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::mat<C, R, T, Q>& matrix) {
	return os << glm::to_string(matrix);
}

template<typename OStream, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, glm::qua<T, Q> quaternion) {
	return os << glm::to_string(quaternion);
}

// Core log macros
#define VI_CORE_TRACE(...)    ::Vi::Log::getCoreLogger()->trace(__VA_ARGS__)
#define VI_CORE_INFO(...)     ::Vi::Log::getCoreLogger()->info(__VA_ARGS__)
#define VI_CORE_WARN(...)     ::Vi::Log::getCoreLogger()->warn(__VA_ARGS__)
#define VI_CORE_ERROR(...)    ::Vi::Log::getCoreLogger()->error(__VA_ARGS__)
#define VI_CORE_CRITICAL(...) ::Vi::Log::getCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define VI_TRACE(...)         ::Vi::Log::getClientLogger()->trace(__VA_ARGS__)
#define VI_INFO(...)          ::Vi::Log::getClientLogger()->info(__VA_ARGS__)
#define VI_WARN(...)          ::Vi::Log::getClientLogger()->warn(__VA_ARGS__)
#define VI_ERROR(...)         ::Vi::Log::getClientLogger()->error(__VA_ARGS__)
#define VI_CRITICAL(...)      ::Vi::Log::getClientLogger()->critical(__VA_ARGS__)
