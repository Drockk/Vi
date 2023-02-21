#pragma once
#include "eventpp/eventqueue.h"

#include "Vi/Event/Event.hpp"

namespace Vi {
    class EventDispatcher {
    public:
        void addListener(EventType type, std::function<void(const EventPointer&)> callback);
        void sendEvent(EventPointer event);
        void process();
    private:
        eventpp::EventQueue<EventType, void(const EventPointer&), EventPolicy> m_Queue;
    };
}
