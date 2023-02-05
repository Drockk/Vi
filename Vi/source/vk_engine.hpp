#pragma once
#include "vk_types.hpp"

#include <GLFW/glfw3.h>

class VulkanEngine {
public:
    bool _isInitialized{ false };
    int _frameNumber{ 0 };

    GLFWwindow* _window{ nullptr };

    VkInstance _instance;
    VkDebugUtilsMessengerEXT _debug_messenger;
    VkPhysicalDevice _chosenGPU;
    VkDevice _device;
    VkSurfaceKHR _surface;

    VkSwapchainKHR _swapchain;
    VkFormat _swapchainImageFormat;
    std::vector<VkImage> _swapchainImages;
    std::vector<VkImageView> _swapchainImageViews;

    void init();
    void cleanup() const;
    void draw();
    void run();

private:
    void init_vulkan();
    void init_swapchain();
};
