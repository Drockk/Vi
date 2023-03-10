#pragma once
#include "Vi/Core/Layer.hpp"

#include <vector>

namespace Vi {
    class LayerStack {
    public:
        LayerStack() = default;
        ~LayerStack();

        void pushLayer(Layer* layer);
        void pushOverlay(Layer* overlay);
        void popLayer(Layer* layer);
        void popOverlay(Layer* overlay);

        [[nodiscard]] std::vector<Layer*>::iterator begin() {
            return m_Layers.begin();
        }

        [[nodiscard]] std::vector<Layer*>::iterator end() {
            return m_Layers.end();
        }

        [[nodiscard]] std::vector<Layer*>::reverse_iterator rbegin() {
            return m_Layers.rbegin();
        }

        [[nodiscard]] std::vector<Layer*>::reverse_iterator rend() {
            return m_Layers.rend();
        }

        [[nodiscard]] std::vector<Layer*>::const_iterator begin() const {
            return m_Layers.begin();
        }

        [[nodiscard]] std::vector<Layer*>::const_iterator end() const {
            return m_Layers.end();
        }

        [[nodiscard]] std::vector<Layer*>::const_reverse_iterator rbegin() const {
            return m_Layers.rbegin();
        }

        [[nodiscard]] std::vector<Layer*>::const_reverse_iterator rend() const {
            return m_Layers.rend();
        }
    private:
        std::vector<Layer*> m_Layers;
        uint32_t m_LayerInsertIndex{ 0 };
    };
}
