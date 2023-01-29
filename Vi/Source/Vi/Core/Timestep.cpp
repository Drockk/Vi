#include "vipch.hpp"
#include "Vi/Core/Timestep.hpp"

namespace Vi {
	Timestep::Timestep(const float time): m_Time(time) {
	}

	Timestep::operator float() const {
		return m_Time;
	}

	float Timestep::getSeconds() const {
		return m_Time;
	}

	float Timestep::getMilliseconds() const {
		return m_Time * 1000.0f;
	}
}
