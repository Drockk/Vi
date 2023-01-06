#pragma once
#include "Vi/Event/Event.hpp"

#include <glfw/glfw3.h>

namespace Vi {
    class Window {
    public:
        using EventCallbackFn = std::function<void(Event&)>;

        Window(const std::string& name, uint32_t width, uint32_t height);
        ~Window();

        static void onUpdate();
        void shutdown();

        void setWindowCallback(const EventCallbackFn& callback);

    private:
        GLFWwindow* m_Window;

        struct WindowData {
            std::string name;
            uint32_t width;
            uint32_t height;
            bool vSync;

            EventCallbackFn eventCallback;
        };

        WindowData m_Data;
    };
}
