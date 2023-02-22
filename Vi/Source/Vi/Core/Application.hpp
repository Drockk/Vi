#pragma once
#include "Vi/Core/Layer.hpp"
#include "Vi/Core/LayerStack.hpp"
#include "Vi/Core/Window.hpp"
#include "Vi/Event/EventDispatcher.hpp"

#include <memory>

namespace Vi {
    class Application {
    public:
        Application(const std::string& name);
        virtual ~Application() = default;

        void init();
        void run();
        void shutdown();

    private:
        void onWindowClose(EventPointer event);

        bool m_Running{ true };
        std::string m_Name;
        Window m_Window;

        std::shared_ptr<EventDispatcher> m_EventDispatcher;
    };
}

std::unique_ptr<Vi::Application> createApplication();
