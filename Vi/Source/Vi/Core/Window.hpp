#pragma once
#include "Vi/Event/Event.hpp"

#include <vulkan/vulkan.hpp>
#include <glfw/glfw3.h>

namespace Vi {
    class Window {
    public:
        using EventCallbackFn = std::function<void(Event&)>;

        Window(const std::string& name, uint32_t width, uint32_t height);
        ~Window() = default;

        static void onUpdate();

        void createSurface(VkSurfaceKHR& surface, VkInstance instance) const;

        void setWindowCallback(const EventCallbackFn& callback);

        const std::string& getName();

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
