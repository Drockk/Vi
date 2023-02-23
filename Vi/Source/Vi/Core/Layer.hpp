#pragma once
#include "Vi/Core/Base.hpp"
#include "Vi/Core/Timestep.hpp"
#include "Vi/Event/Event.hpp"

namespace Vi {
    class Layer {
    public:
        Layer(std::string layerName = "Layer");
        virtual ~Layer() = default;

        virtual void onAttach() {}
        virtual void onDetach() {}
        virtual void onUpdate(Timestep ts) {}
        virtual void onImGuiRender() {}
        virtual void onEvent(Event& event) {}

        [[nodiscard]] const std::string& getName() const {
            return m_DebugName;
        }

    protected:
        std::string m_DebugName;
    };
}
