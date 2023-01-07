#include "vipch.hpp"
#include "Vi/Renderer/Context.hpp"

#include <VkBootstrap.h>

namespace Vi {
	void Context::init(const std::shared_ptr<Window>& window) {
		vkb::InstanceBuilder builder;
		auto instRet = builder.set_app_name("Vi Engine")
			.set_engine_name("Vi Engine")
			.require_api_version(1, 1, 0)
			.request_validation_layers(true)
			.use_default_debug_messenger()
			.build();

		ASSERT_CORE_LOG(instRet.has_value(), "Failed to create Vulkan instance.");

		auto vkbInstance = instRet.value();

		VkSurfaceKHR surface{ nullptr };
		window->createSurface(surface, vkbInstance.instance);

		vkb::PhysicalDeviceSelector selector{ vkbInstance };
		auto physRet = selector.set_surface(surface)
			.set_minimum_version(1, 1)
			.require_dedicated_transfer_queue()
			.select();

		ASSERT_CORE_LOG(physRet.has_value(), "Failed to select physical device");

		auto physicDevice = physRet.value();
		vkb::DeviceBuilder deviceBuilder{ physicDevice };
		auto devRet = deviceBuilder.build();
		ASSERT_CORE_LOG(devRet.has_value(), "Failed to create Vulkan device");
		auto vkbDevice = devRet.value();
		auto device = vkbDevice.device;

		auto graphicsQueueRet = vkbDevice.get_queue(vkb::QueueType::graphics);
		ASSERT_CORE_LOG(graphicsQueueRet.has_value(), "Failed to get graphics queue");
		auto graphicsQueue = graphicsQueueRet.value();
	}
}
