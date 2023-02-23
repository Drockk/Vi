#pragma once

#include "Vi/Core/Base.hpp"
#include "Vi/Event/Event.hpp"

#include <sstream>

namespace Vi {
    struct WindowProperties {
        std::string Title;
        uint32_t Width;
        uint32_t Height;

        WindowProperties(const std::string& title = "Vi Engine", uint32_t width = 1600, uint32_t height = 900): Title(title), Width(width), Height(height) {
        }
    };

    // Interface representing a desktop system based Window
    class Window {
    public:
        using EventCallbackFn = std::function<void(Event&)>;
        virtual ~Window() = default;

        virtual void onUpdate() = 0;
        virtual uint32_t getWidth() const = 0;
        virtual uint32_t getHeight() const = 0;

        // Window attributes
        virtual void setEventCallback(const EventCallbackFn& callback) = 0;
        virtual void setVSync(bool enabled) = 0;
        virtual bool isVSync() const = 0;

        virtual void* getNativeWindow() const = 0;

        static Scope<Window> create(const WindowProperties& props = WindowProperties());
    };
}
