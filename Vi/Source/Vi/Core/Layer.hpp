//Initially inspired from https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/Core/Layer.h
#pragma once
#include "Vi/Core/Timestep.hpp"
#include "Vi/Event/Event.hpp"

namespace Vi {
	class Layer {
	public:
		Layer() = delete;
		Layer(const std::string& name = "Layer");
		virtual ~Layer() = default;

		virtual void onAttach() {}
		virtual void onDetach() {}
		virtual void onUpdate(const Timestep& timestep) {}
		virtual void onEvent(Event& event) {}

		[[nodiscard]] const std::string& getName() const;
	protected:
		std::string m_DebugName;
	};
}
