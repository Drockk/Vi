#pragma once

#include "ViTypes.hpp"

namespace viinit {
	VkCommandPoolCreateInfo commandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);

	VkCommandBufferAllocateInfo commandBufferAllocateInfo(VkCommandPool pool, uint32_t count = 1, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule shaderModule);

	VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo();

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo(VkPrimitiveTopology topology);

	VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo(VkPolygonMode polygonMode);

	VkPipelineMultisampleStateCreateInfo multisamplingStateCreateInfo();

	VkPipelineColorBlendAttachmentState colorBlendAttachmentState();

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo();

	VkFenceCreateInfo fenceCreateInfo(VkFenceCreateFlags flags = 0);
	VkSemaphoreCreateInfo semaphoreCreateInfo(VkSemaphoreCreateFlags flags = 0);
}