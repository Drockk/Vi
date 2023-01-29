#pragma once

#include "Vi/Core/Window.hpp"
#include "Vi/Event/ApplicationEvent.hpp"

#include "Vi/Renderer/Renderer.hpp"

namespace Vi {
    class Application {
    public:
        Application(std::string applicationName = "ViApp", uint32_t width = 1600, uint32_t height = 900);
        ~Application() = default;

        void init();
        void run();
        void shutdown() const;

        void onEvent(Event&);
        bool onWindowCloseEvent(WindowCloseEvent& event);

    private:
        Renderer m_Renderer;
        std::shared_ptr<Window> m_Window;

        std::string m_ApplicationName;

        uint32_t m_Width;
        uint32_t m_Height;

        bool m_Running{ true };
    };
}

std::unique_ptr<Vi::Application> createApplication();
