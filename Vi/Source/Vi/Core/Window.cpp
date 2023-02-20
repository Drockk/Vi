#include "vipch.hpp"
#include "Vi/Core/Window.hpp"

namespace Vi {
    Window::Window(std::string title, uint32_t width, uint32_t height): m_Title(std::move(title)), m_Width(width), m_Height(height), m_Window(nullptr) {
    }

    Window::~Window() {
        if (m_Window) {
            glfwDestroyWindow(m_Window);
        }

        glfwTerminate();
    }

    void Window::onUpdate() {
        glfwPollEvents();
    }

    void Window::init() {
        if (not glfwInit()) {
            VI_CORE_CRITICAL("Cannot initialize GLFW");
            std::terminate();
        }

        glfwSetErrorCallback([](int code, const char* description) {
            VI_CORE_ERROR("GLFW Error({}): {}", code, description);
        });

        int major, minor, revision;
        glfwGetVersion(&major, &minor, &revision);
        VI_CORE_TRACE("GLFW version: {}.{}.{}", major, minor, revision);

        m_Window = glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), nullptr, nullptr);
        if (not m_Window) {
            VI_CORE_CRITICAL("Cannot create Window");
            glfwTerminate();
            std::terminate();
        }

        glfwMakeContextCurrent(m_Window);

        glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
            
        });
    }
}
