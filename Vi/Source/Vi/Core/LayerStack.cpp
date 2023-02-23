#include "vipch.hpp"
#include "Vi/Core/LayerStack.hpp"

namespace Vi {
    LayerStack::~LayerStack() {
		for (auto* layer : m_Layers) {
			layer->onDetach();
			delete layer;
		}
    }

	void LayerStack::pushLayer(Layer* layer) {
		m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
		m_LayerInsertIndex++;
	}

	void LayerStack::pushOverlay(Layer* overlay) {
		m_Layers.emplace_back();
	}

	void LayerStack::popLayer(Layer* layer)
	{
		if (const auto it = std::find(m_Layers.begin(), m_Layers.begin() + m_LayerInsertIndex, layer); it != m_Layers.begin() + m_LayerInsertIndex)
		{
			layer->onDetach();
			m_Layers.erase(it);
			m_LayerInsertIndex--;
		}
	}

	void LayerStack::popOverlay(Layer* overlay)
	{
		if (const auto it = std::find(m_Layers.begin() + m_LayerInsertIndex, m_Layers.end(), overlay); it != m_Layers.end())
		{
			overlay->onDetach();
			m_Layers.erase(it);
		}
	}
}
