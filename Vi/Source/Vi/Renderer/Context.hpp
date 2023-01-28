#pragma once
#include <VkBootstrap.h>

#include "Vi/Core/Window.hpp"

namespace Vi {
	class Context {
	public:
		void init(const std::shared_ptr<Window>& window);
		void shutdown() const;

	private:
		void initInstance(const std::string& name);
		void initSurface(const std::shared_ptr<Window>& window);
		void initDevice();
		void initSwapchain();

		void getGraphicsQueue();
		void getPresentQueue();

		void createRenderpass();
		void createGraphicsPipeline();
		static std::vector<char> readFile(const std::string& filename);
		[[nodiscard]] VkShaderModule createShaderModule(const std::vector<char>& code) const;
		void createFramebuffers();
		void createCommandPool();
		void createCommandBuffers();

		VkSurfaceKHR m_Surface{nullptr};
		vkb::Instance m_Instance;
		vkb::Device m_Device;
		vkb::Swapchain m_Swapchain;

		VkQueue m_GraphicsQueue{};
		VkQueue m_PresentQueue{};
		VkRenderPass m_RenderPass{};
		VkPipelineLayout m_PipelineLayout{};
		VkPipeline m_GraphicsPipeline{};
		std::vector<VkImage> m_SwapchainImages;
		std::vector<VkImageView> m_SwapchainImageViews;
		std::vector<VkFramebuffer> m_Framebuffers;
		VkCommandPool m_CommandPool{};
		std::vector<VkCommandBuffer> m_CommandBuffers;
	};
}
