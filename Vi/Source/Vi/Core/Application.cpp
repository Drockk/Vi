#include "vipch.hpp"
#include "Application.hpp"

namespace Vi {
    void Application::init() {
        Log::init();

        m_Window = Window("Test", 1600, 900);
        m_Window.init();
    }

    void Application::run() {
        while (m_Running) {
            m_Window.onUpdate();
        }
    }

    void Application::shutdown() {
    }
}
