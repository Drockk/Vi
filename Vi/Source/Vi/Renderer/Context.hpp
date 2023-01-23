#pragma once
#include <VkBootstrap.h>

#include "Vi/Core/Window.hpp"

namespace Vi {
	class Context {
	public:
		void init(const std::shared_ptr<Window>& window);

	private:
		void initInstance(const std::string& name);
		void initSurface(const std::shared_ptr<Window>& window);
		void initDevice();
		void initSwapchain();

		void getGraphicsQueue();
		void getPresentQueue();

		void createRenderpass();
		void createGraphicsPipeline();

		VkSurfaceKHR m_Surface{nullptr};
		vkb::Instance m_Instance;
		vkb::Device m_Device;
		vkb::Swapchain m_Swapchain;

		VkQueue m_GraphicsQueue{};
		VkQueue m_PresentQueue{};
		VkRenderPass m_RenderPass{};
		VkPipelineLayout m_PipelineLayout;
	};
}
