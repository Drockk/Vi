#include "vipch.hpp"
#include "Application.hpp"

#include "Vi/Event/ApplicationEvent.hpp"

namespace Vi {
    Application::Application(const std::string& name): m_Name(name) {
    }

    void Application::init() {
        Log::init();

        m_EventDispatcher = std::make_shared<EventDispatcher>();
        m_EventDispatcher->addListener(EventType::WindowClose, VI_BIND_EVENT_FN(Application::onWindowClose));

        m_Window = Window({ m_Name, 1600, 900, m_EventDispatcher });
        m_Window.init();
    }

    void Application::run() {
        while (m_Running) {
            m_Window.onUpdate();
            m_EventDispatcher->process();
        }
    }

    void Application::shutdown() {
    }

    void Application::onWindowClose(EventPointer event) {
        m_Running = false;
    }
}
