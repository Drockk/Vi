#include "vipch.hpp"
#include "Vi/Core/Application.hpp"

namespace Vi {
	Application::Application(std::string applicationName, const uint32_t width, const uint32_t height): m_ApplicationName(std::move(applicationName)), m_Width(width), m_Height(height) {
		
	}

	void Application::init() {
		Log::init();

		VI_CORE_DEBUG("Application initzialization");

		m_Window = std::make_unique<Window>(m_ApplicationName, m_Width, m_Height);
	}

	void Application::run() const {
		VI_CORE_DEBUG("Application running");

		while (m_Running) {
			m_Window->onUpdate();
		}
	}

	void Application::shutdown() const {
		VI_CORE_DEBUG("Application shuting down");

		m_Window->shutdown();
	}
}
