#pragma once
#include "Vi/Core/Log.hpp"

namespace Vi {
	inline void assertCoreLog(const bool condition, const std::string& message) {
		if (!condition) {
			VI_CORE_CRITICAL(message);
			assert(condition);
		}
	}
}

#define  ASSERT_CORE_LOG(condition, message) ::Vi::assertCoreLog(condition, message)
