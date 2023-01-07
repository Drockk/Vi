#pragma once
#include "Vi/Core/Window.hpp"

namespace Vi {
	class Context {
	public:
		void init(const std::shared_ptr<Window>& window);
	};
}
