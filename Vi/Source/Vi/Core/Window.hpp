#pragma once
#include <GLFW/glfw3.h>

#include <string>

namespace Vi {
    class Window
    {
    public:
        Window() = default;
        Window(std::string title, uint32_t width, uint32_t height);
        ~Window();

        [[noreturn]] void init();
        [[noreturn]] void onUpdate();

    private:
        std::string m_Title;
        uint32_t m_Width;
        uint32_t m_Height;

        GLFWwindow* m_Window;
    };
}
