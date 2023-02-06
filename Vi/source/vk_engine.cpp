#include "vk_engine.hpp"

#include <vk_types.hpp>
#include <vk_init.hpp>
#include "VkBootstrap.h"

#include <iostream>

constexpr uint32_t WINDOW_WIDTH{ 1600 };
constexpr uint32_t WINDOW_HEIGHT{ 900 };

#define VK_CHECK(x)                                                         \
    do {                                                                    \
        VkResult err = x;                                                   \
        if(err) {                                                           \
            std::cerr << "Detected Vulkan error: " << err << "\n";          \
            abort();                                                        \
        }                                                                   \
    } while(0)

void VulkanEngine::init() {

    glfwSetErrorCallback([](int error, const char* description) {
        std::cerr << std::format("Error: {} {}\n", error, description);
    });

    if (!glfwInit()) {
        throw std::runtime_error("Cannot initialize GLFW");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    _window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Vi", nullptr, nullptr);
    if (!_window) {
        glfwTerminate();
        throw std::runtime_error("Cannot create window");
    }

    glfwSetKeyCallback(_window, [](GLFWwindow* window, int key, [[maybe_unused]] int scancode, int action, [[maybe_unused]] int mods) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        if (action == GLFW_PRESS) {
            std::cout << std::format("Pressed key: {}\n", key);
        }
    });

    initVulkan();
    initCommands();

    //everything went fine
    _isInitialized = true;
}

void VulkanEngine::cleanup() const {
    if (_isInitialized) {
        vkDestroyCommandPool(_device, _commandPool, nullptr);

        vkDestroySwapchainKHR(_device, _swapchain, nullptr);

        //destroy swapchain resources
        for (const auto& swapchainImageView : _swapchainImageViews) {
            vkDestroyImageView(_device, swapchainImageView, nullptr);
        }

        vkDestroyDevice(_device, nullptr);
        vkDestroySurfaceKHR(_instance, _surface, nullptr);
        vkb::destroy_debug_utils_messenger(_instance, _debug_messenger);
        vkDestroyInstance(_instance, nullptr);

        glfwDestroyWindow(_window);
        glfwTerminate();
    }
}

void VulkanEngine::draw() {

}

void VulkanEngine::run() {
    while (!glfwWindowShouldClose(_window)) {
        glfwPollEvents();

        draw();
    }
}

void VulkanEngine::initVulkan() {
    //Instance
    vkb::InstanceBuilder builder;

    auto inst_ret = builder.set_app_name("Example Vulkan Application")
        .request_validation_layers(true)
        .require_api_version(1, 1, 0)
        .use_default_debug_messenger()
        .build();

    auto vkb_inst = inst_ret.value();
    _instance = vkb_inst.instance;
    _debug_messenger = vkb_inst.debug_messenger;

    //Device
    glfwCreateWindowSurface(_instance, _window, nullptr, &_surface);

    vkb::PhysicalDeviceSelector selector{ vkb_inst };
    auto physicalDevice = selector
        .set_minimum_version(1, 1)
        .set_surface(_surface)
        .select()
        .value();

    vkb::DeviceBuilder deviceBuilder{ physicalDevice };
    auto vkbDevice = deviceBuilder.build().value();

    _device = vkbDevice.device;
    _chosenGPU = physicalDevice.physical_device;

    _graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
    _graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
}

void VulkanEngine::initSwapchain() {
    vkb::SwapchainBuilder swapchainBuilder{ _chosenGPU,_device,_surface };

    auto vkbSwapchain = swapchainBuilder
        .use_default_format_selection()
        //use vsync present mode
        .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
        .set_desired_extent(WINDOW_WIDTH, WINDOW_HEIGHT)
        .build()
        .value();

    //store swapchain and its related images
    _swapchain = vkbSwapchain.swapchain;
    _swapchainImages = vkbSwapchain.get_images().value();
    _swapchainImageViews = vkbSwapchain.get_image_views().value();

    _swapchainImageFormat = vkbSwapchain.image_format;
}

void VulkanEngine::initCommands() {
    const VkCommandPoolCreateInfo commandPoolInfo = Vkinit::commandPoolCreateInfo(_graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    VK_CHECK(vkCreateCommandPool(_device, &commandPoolInfo, nullptr, &_commandPool));

    const VkCommandBufferAllocateInfo cmdAllocInfo = Vkinit::commandBufferAllocateInfo(_commandPool, 1);

    VK_CHECK(vkAllocateCommandBuffers(_device, &cmdAllocInfo, &_mainCommandBuffer));
}
