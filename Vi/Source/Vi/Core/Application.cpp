#include "vipch.hpp"
#include "Vi/Core/Application.hpp"
#include "Vi/Core/Input.hpp"
#include "Vi/Core/Log.hpp"
#include "Vi/Scripting/ScriptEngine.hpp"
#include "Vi/Renderer/Renderer.hpp"
#include "Vi/Utils/PlatformUtils.hpp"

namespace Vi {
	Application* Application::s_Instance{ nullptr };

	Application::Application(const ApplicationSpecification& specification): m_Specification(specification) {
		VI_PROFILE_FUNCTION();

		VI_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		//Set working directory here
		if (!m_Specification.WorkingDirectory.empty()) {
			std::filesystem::current_path(m_Specification.WorkingDirectory);
		}

		m_Window = Window::create(WindowProperties(m_Specification.Name));
		m_Window->setEventCallback(VI_BIND_EVENT_FN(Application::onEvent));

		Renderer::init();

		m_ImGuiLayer = new ImGuiLayer();
		pushOverlay(m_ImGuiLayer);
	}
}
