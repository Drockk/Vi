#pragma once

#include "Vi/Core/Base.hpp"
#include "Vi/Core/Log.hpp"
#include <filesystem>

#ifdef VI_ENABLE_ASSERTS

// Alteratively we could use the same "default" message for both "WITH_MSG" and "NO_MSG" and
// provide support for custom formatting by concatenating the formatting string instead of having the format inside the default message
#define VI_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { VI##type##ERROR(msg, __VA_ARGS__); VI_DEBUGBREAK(); } }
#define VI_INTERNAL_ASSERT_WITH_MSG(type, check, ...) VI_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
#define VI_INTERNAL_ASSERT_NO_MSG(type, check) VI_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", VI_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

#define VI_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
#define VI_INTERNAL_ASSERT_GET_MACRO(...) VI_EXPAND_MACRO( VI_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, VI_INTERNAL_ASSERT_WITH_MSG, VI_INTERNAL_ASSERT_NO_MSG) )

// Currently accepts at least the condition and one additional parameter (the message) being optional
#define VI_ASSERT(...) VI_EXPAND_MACRO( VI_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
#define VI_CORE_ASSERT(...) VI_EXPAND_MACRO( VI_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )
#else
#define VI_ASSERT(...)
#define VI_CORE_ASSERT(...)
#endif
