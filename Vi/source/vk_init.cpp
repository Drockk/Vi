#include "vk_init.hpp"

VkCommandPoolCreateInfo Vkinit::commandPoolCreateInfo(const uint32_t queueFamilyIndex, const VkCommandPoolCreateFlags flags) {
	VkCommandPoolCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	info.pNext = nullptr;

	info.queueFamilyIndex = queueFamilyIndex;
	info.flags = flags;
	return info;
}

VkCommandBufferAllocateInfo Vkinit::commandBufferAllocateInfo(const VkCommandPool pool, const uint32_t count, const VkCommandBufferLevel level) {
	VkCommandBufferAllocateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	info.pNext = nullptr;

	info.commandPool = pool;
	info.commandBufferCount = count;
	info.level = level;
	return info;
}
