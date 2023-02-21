#include "vipch.hpp"
#include "Application.hpp"

#include "Vi/Event/ApplicationEvent.hpp"

namespace Vi {
    void Application::init() {
        Log::init();

        m_Window = Window("Test", 1600, 900);
        m_Window.init();

        m_EventDispatcher.addListener(EventType::WindowClose, [](EventPointer event) {
            VI_CORE_INFO("AAAAA");
        });
    }

    void Application::run() {
        while (m_Running) {
            m_Window.onUpdate();
            m_EventDispatcher.process();
            m_EventDispatcher.sendEvent(std::make_shared<WindowCloseEvent>());
        }
    }

    void Application::shutdown() {
    }
}
