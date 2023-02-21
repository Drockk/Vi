#pragma once
#include "Vi/Event/EventDispatcher.hpp"
#include <GLFW/glfw3.h>

#include <string>

namespace Vi {
    struct WindowParameters {
        std::string Title;
        uint32_t Width;
        uint32_t Height;
        std::shared_ptr<EventDispatcher> EventDispatcher;
    };

    class Window
    {
    public:
        Window() = default;
        Window(WindowParameters parameters);
        ~Window();

        [[noreturn]] void init();
        [[noreturn]] void onUpdate();

    private:
        WindowParameters m_Parameters;

        GLFWwindow* m_Window;
    };
}
