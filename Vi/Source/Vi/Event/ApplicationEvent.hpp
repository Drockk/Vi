#pragma once
#include "Vi/Event/Event.hpp"

namespace Vi {
    class WindowCloseEvent: public Event {
    public:
        WindowCloseEvent() : Event(EventType::WindowClose) {

        }

        ~WindowCloseEvent() override = default;
    };
}
