#include "vipch.hpp"
#include "Vi/Event/EventDispatcher.hpp"

namespace Vi {
	void EventDispatcher::addListener(EventType type, std::function<void(const EventPointer&)> callback) {
		m_Queue.appendListener(type, callback);
	}

	void EventDispatcher::sendEvent(EventPointer event) {
		m_Queue.enqueue(event);
	}

	void EventDispatcher::process() {
		m_Queue.process();
	}
}
