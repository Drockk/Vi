#include "vipch.hpp"
#include "Vi/Core/ShutdownQueue.hpp"

namespace Vi {
	void ShutdownQueue::addToQueue(std::function<void()>&& function) {
		m_Deletors.push(function);
	}

	void ShutdownQueue::flush() {
		while (!m_Deletors.empty()) {
			auto it = m_Deletors.top();
			(it)();
			m_Deletors.pop();
		}
	}
}
