#include "vipch.hpp"
#include "Vi/Core/Application.hpp"

namespace Vi {
	Application::Application(std::string applicationName, const uint32_t width, const uint32_t height): m_ApplicationName(std::move(applicationName)), m_Width(width), m_Height(height) {
		
	}

	void Application::init() {
		Log::init();

		VI_CORE_DEBUG("Application initzialization");

		m_Window = std::make_shared<Window>(m_ApplicationName, m_Width, m_Height);
		m_Window->setWindowCallback(VI_BIND_EVENT_FN(onEvent));

		Context context;
		context.init(m_Window);
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

	void Application::onEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.dispatch<WindowCloseEvent>(VI_BIND_EVENT_FN(Application::onWindowCloseEvent));
	}

	bool Application::onWindowCloseEvent(WindowCloseEvent& event) {
		m_Running = false;
		return true;
	}
}
