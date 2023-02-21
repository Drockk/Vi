#include "vipch.hpp"
#include "Vi/Core/Window.hpp"
#include "Vi/Event/ApplicationEvent.hpp"

namespace Vi {
    Window::Window(WindowParameters parameters): m_Parameters(parameters), m_Window(nullptr) {
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

        m_Window = glfwCreateWindow(m_Parameters.Width, m_Parameters.Height, m_Parameters.Title.c_str(), nullptr, nullptr);
        if (not m_Window) {
            VI_CORE_CRITICAL("Cannot create Window");
            glfwTerminate();
            std::terminate();
        }

        glfwMakeContextCurrent(m_Window);
        glfwSetWindowUserPointer(m_Window, &m_Parameters);

        glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
            WindowParameters& data = *static_cast<WindowParameters*>(glfwGetWindowUserPointer(window));
            data.EventDispatcher->sendEvent(std::make_shared<WindowCloseEvent>());
        });
    }
}
