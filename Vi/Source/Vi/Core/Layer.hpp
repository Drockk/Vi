#pragma once
#include <string>

namespace Vi {
    class Layer
    {
    public:
        Layer(const std::string& layerName);
        virtual ~Layer() = default;

        virtual void onAttach();
        virtual void onUpdate();
        virtual void onDetach();

    private:
        std::string m_DebugName;
    };
}
