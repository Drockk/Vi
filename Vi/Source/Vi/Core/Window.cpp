#include "vipch.hpp"
#include "Vi/Core/Window.hpp"
#include "Vi/Event/ApplicationEvent.hpp"

namespace Vi {
    Window::Window(const std::string& name, const uint32_t width, const uint32_t height) : m_Data{ name, width, height, true, nullptr } {
        VI_CORE_TRACE("Creating GLFW Window ({0}, {1})", width, height);

        ASSERT_CORE_LOG(glfwInit(), "Cannot initialize GLFW");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        m_Window = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), name.c_str(), nullptr, nullptr);
        if(!m_Window) {
            glfwTerminate();
            ASSERT_CORE_LOG(true, "Cannot create GLFW window");
        }

        glfwSetWindowUserPointer(m_Window, &m_Data);

        glfwSetErrorCallback([](int code, const char* description) {
            VI_CORE_ERROR("GLFW Error({0}): {1}", code, description);
        });

        glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
            const auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

            VI_CORE_DEBUG("GLFW Window Close Event");
            WindowCloseEvent event;
            data.eventCallback(event);
        });
    }

    Window::~Window() {
        shutdown();
    }

    void Window::onUpdate() {
        glfwPollEvents();
    }

    void Window::shutdown() {
        if (m_Window) {
            glfwDestroyWindow(m_Window);
            m_Window = nullptr;
            glfwTerminate();
        }
    }

    void Window::createSurface(VkSurfaceKHR& surface, const VkInstance instance) const {
        const auto result = glfwCreateWindowSurface(instance, m_Window, nullptr, &surface);
        ASSERT_CORE_LOG(result == VK_SUCCESS, "Cannot create Surface");
    }

    void Window::setWindowCallback(const EventCallbackFn& callback) {
        m_Data.eventCallback = callback;
    }
}
