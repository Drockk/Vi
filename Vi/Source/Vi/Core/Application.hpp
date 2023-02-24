#pragma once

#include "Vi/Core/Base.hpp"
#include "Vi/Core/LayerStack.hpp"
#include "Vi/Core/Timestep.hpp"
#include "Vi/Core/Window.hpp"
#include "Vi/Event/Event.hpp"
#include "Vi/Event/ApplicationEvent.hpp"
#include "Vi/ImGui/ImGuiLayer.hpp"

int main(int argc, char** argv);

namespace Vi {
    struct ApplicationCommandLineArgs {
        int Count{ 0 };
        char** Args{ nullptr };

        const char* operator[](int index) const {
            VI_CORE_ASSERT(index < Count);
            return Args[index];
        }
    };

    struct ApplicationSpecification {
        std::string Name{ "Vi Application" };
        std::string WorkingDirectory;
        ApplicationCommandLineArgs CommandLineArgs;
    };

    class Application {
    public:
        Application(const ApplicationSpecification& specification);
        virtual ~Application();

        void onEvent(Event& e);

        void pushLayer(Layer* layer);
        void pushOverlay(Layer* layer);

        Window& getWindow() {
            return *m_Window;
        }

        void close();

        ImGuiLayer* getImGuiLayer() {
            return m_ImGuiLayer;
        }

        static Application& get() {
            return *s_Instance;
        }

        const ApplicationSpecification& getSpecification() const {
            return m_Specification;
        }

        void submitToMainThread(const std::function<void()>& function);

    private:
        void run();

        bool onWindowClose(WindowCloseEvent& event);
        bool onWindowResize(WindowResizeEvent& event);

        void executeMainThreadQueue(const std::function<void()>& function);

        ApplicationSpecification m_Specification;
        Scope<Window> m_Window;
        ImGuiLayer* m_ImGuiLayer;
        bool m_Running{ true };
        bool m_Minimized{ false };
        LayerStack m_LayerStack;
        float m_LastFrameTime{ 0.0f };

        std::vector<std::function<void()>> m_MainThreadQueue;
        std::mutex m_MainThreadQueueMutex;

        static Application* s_Instance;
        friend int ::main(int argc, char** argv);
    };

    // To be defined in CLIENT
    Application* createApplication(ApplicationCommandLineArgs args);
}
