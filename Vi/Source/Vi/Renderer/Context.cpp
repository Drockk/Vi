#include "vipch.hpp"
#include "Vi/Renderer/Context.hpp"

namespace Vi {
	void Context::init(const std::shared_ptr<Window>& window) {
		initInstance(window->getName());
		initSurface(window);
		initDevice();
		initSwapchain();

		getGraphicsQueue();
		getPresentQueue();
	}

	void Context::initInstance(const std::string& name) {
		vkb::InstanceBuilder builder;
		auto instRet = builder.set_app_name(name.c_str())
			.set_engine_name("Vi Engine")
			.require_api_version(1, 1, 0)
			.request_validation_layers(true)
			.set_debug_callback([](VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, [[maybe_unused]] void* pUserData) -> VkBool32 {
				auto type = vkb::to_string_message_type(messageType);

				switch (messageSeverity) {
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
					VI_CORE_TRACE("[{0}]: {1}", type, pCallbackData->pMessage);
					break;
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
					VI_CORE_INFO("[{0}]: {1}", type, pCallbackData->pMessage);
					break;
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
					VI_CORE_WARN("[{0}]: {1}", type, pCallbackData->pMessage);
					break;
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
					VI_CORE_ERROR("[{0}]: {1}", type, pCallbackData->pMessage);
					break;
				default:
					VI_CORE_DEBUG("[{0}]: {1}", type, pCallbackData->pMessage);
					break;
				}

				return VK_FALSE;
			})
			.build();

		ASSERT_CORE_LOG(instRet.has_value(), "Failed to create Vulkan instance.");

		m_Instance = instRet.value();

		ShutdownQueue::addToQueue([instance = m_Instance] {
			destroy_instance(instance);
		});
	}

	void Context::initSurface(const std::shared_ptr<Window>& window) {
		window->createSurface(m_Surface, m_Instance.instance);

		ShutdownQueue::addToQueue([instance = m_Instance, surface = m_Surface] {
			destroy_surface(instance, surface);
		});
	}

	void Context::initDevice() {
		vkb::PhysicalDeviceSelector selector{ m_Instance };
		auto physRet = selector.set_surface(m_Surface)
			.set_minimum_version(1, 1)
			.require_dedicated_transfer_queue()
			.select();

		ASSERT_CORE_LOG(physRet.has_value(), "Failed to select physical device");

		const auto physicDevice = physRet.value();
		const vkb::DeviceBuilder deviceBuilder{ physicDevice };
		auto devRet = deviceBuilder.build();
		ASSERT_CORE_LOG(devRet.has_value(), "Failed to create Vulkan device");
		m_Device = devRet.value();

		ShutdownQueue::addToQueue([device = m_Device] {
			destroy_device(device);
		});
	}

	void Context::initSwapchain() {
		vkb::SwapchainBuilder swapchainBuilder{ m_Device };
		auto swapchainRet = swapchainBuilder.set_old_swapchain(m_Swapchain).build();
		ASSERT_CORE_LOG(swapchainRet.has_value(), "Failed to build swapchain");
		destroy_swapchain(m_Swapchain);

		m_Swapchain = swapchainRet.value();

		ShutdownQueue::addToQueue([swapchain = m_Swapchain] {
			destroy_swapchain(swapchain);
		});
	}

	void Context::getGraphicsQueue() {
		auto graphicsQueueRet = m_Device.get_queue(vkb::QueueType::graphics);
		ASSERT_CORE_LOG(graphicsQueueRet.has_value(), "Failed to get graphics queue");
		m_GraphicsQueue = graphicsQueueRet.value();
	}

	void Context::getPresentQueue() {
		auto presentQueueRet = m_Device.get_queue(vkb::QueueType::present);
		ASSERT_CORE_LOG(presentQueueRet.has_value(), "Failed to get present queue");
		m_PresentQueue = presentQueueRet.value();
	}
}
