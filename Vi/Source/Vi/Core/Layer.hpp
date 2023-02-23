#pragma once
#include <string>

namespace Vi {
    class Layer
    {
    public:
        Layer(std::string layerName = "Layer");
        virtual ~Layer() = default;

        virtual void onAttach();
        virtual void onUpdate();
        virtual void onDetach();

        [[nodiscard]] const std::string& getName() const {
            return m_DebugName;
        }

    protected:
        std::string m_DebugName;
    };
}
