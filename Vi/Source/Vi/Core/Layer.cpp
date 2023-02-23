#include "vipch.hpp"
#include "Vi/Core/Layer.hpp"

namespace Vi {
    Layer::Layer(std::string layerName): m_DebugName(std::move(layerName)) {
    }

    void Layer::onAttach() {
    }

    void Layer::onUpdate() {
    }

    void Layer::onDetach() {
    }
}
