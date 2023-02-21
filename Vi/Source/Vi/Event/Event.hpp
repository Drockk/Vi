#pragma once
#include <memory>

namespace Vi {
    enum class EventType {
        None = 0,
        WindowClose,
    };

    class Event {
    public:
        explicit Event(const EventType type) : m_Type(type) {

        }

        virtual ~Event() = default;

        EventType getType() const {
            return m_Type;
        }

    private:
        EventType m_Type;
    };

    using EventPointer = std::shared_ptr<Event>;

    struct EventPolicy {
        static EventType getEvent(const EventPointer& event) {
            return event->getType();
        }
    };
}
