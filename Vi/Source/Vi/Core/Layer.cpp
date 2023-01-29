#include "vipch.hpp"
#include "Vi/Core/Layer.hpp"

namespace Vi {
	Layer::Layer(const std::string& name): m_DebugName(name) {
		
	}

	const std::string& Layer::getName() const {
		return m_DebugName;
	}
}
