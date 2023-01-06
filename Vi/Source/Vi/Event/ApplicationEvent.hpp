#pragma once
#include "Vi/Event/Event.hpp"

namespace Vi {
    class WindowCloseEvent: public Event{
    public:
        WindowCloseEvent() = default;

        EVENT_CLASS_TYPE(WindowClose)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)
    };
}
