#pragma once
#include "Vi/Renderer/Context.hpp"
#include "Vi/Core/Window.hpp"

namespace Vi {
	class Renderer {
	public:
		Renderer() = default;
		~Renderer() = default;

		void init(const std::shared_ptr<Window>& window);
		void draw();
		void shutdown() const;

	private:
		Context m_Context;
	};
}
