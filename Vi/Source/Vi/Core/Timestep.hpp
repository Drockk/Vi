//Initialy inspired from https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/Core/Timestep.h
#pragma once

namespace Vi {
	class Timestep {
	public:
		Timestep(float time = 0.0f);

		operator float() const;

		[[nodiscard]] float getSeconds() const;
		[[nodiscard]] float getMilliseconds() const;

	private:
		float m_Time;
	};
}
