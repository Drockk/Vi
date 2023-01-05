#pragma once

#include <glfw/glfw3.h>

namespace Vi {
    class Window {
    public:
        Window(const std::string& name, uint32_t width, uint32_t height);
        ~Window();

        static void onUpdate();
        void shutdown();

    private:
        GLFWwindow* m_Window;

        struct WindowData {
            std::string name;
            uint32_t width;
            uint32_t height;
            //TODO Add event callback
        };

        WindowData m_Data;
    };
}
