#pragma once
#include <stack>

namespace Vi {
	class ShutdownQueue {
	public:
		ShutdownQueue() = default;
		~ShutdownQueue() = default;

		static void addToQueue(std::function<void()>&& function);

		static void flush();

	private:
		inline static std::stack<std::function<void()>> m_Deletors;
	};
}
