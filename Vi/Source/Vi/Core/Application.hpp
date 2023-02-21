#pragma once
#include "Vi/Core/Window.hpp"
#include "Vi/Event/EventDispatcher.hpp"

#include <memory>

namespace Vi {
    class Application {
    public:
        Application() = default;
        virtual ~Application() = default;

        void init();
        void run();
        void shutdown();

    private:
        void onWindowClose(EventPointer event);

        bool m_Running{ true };
        Window m_Window;

        std::shared_ptr<EventDispatcher> m_EventDispatcher;
    };
}

std::unique_ptr<Vi::Application> createApplication();
