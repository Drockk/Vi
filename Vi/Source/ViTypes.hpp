#pragma once
#include <vulkan/vulkan.hpp>

#include <vk_mem_alloc.h>

struct AllocatedBuffer {
    VkBuffer buffer;
    VmaAllocation allocation;
};