#include "vipch.hpp"
#include "Vi/Renderer/Renderer.hpp"

namespace Vi {
	void Renderer::init(const std::shared_ptr<Window>& window) {
		m_Context.init(window);
	}

	void Renderer::draw() {
		m_Context.draw();
	}

	void Renderer::shutdown() const {
		m_Context.shutdown();
	}
}
