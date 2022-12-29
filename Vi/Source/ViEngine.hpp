#pragma once

#include "ViTypes.hpp"

//bootstrap library
#include "VkBootstrap.h"

//GLFW
#include <GLFW/glfw3.h>

#include <iostream>
#include <functional>
#include <deque>

#include "ViMesh.hpp"

#define VK_CHECK(x)                                                     \
    do                                                                  \
    {                                                                   \
        VkResult err = x;                                               \
        if (err)                                                        \
        {                                                               \
            std::cout <<"Detected Vulkan error: " << err << std::endl;  \
            abort();                                                    \
        }                                                               \
    } while (0)

struct DeletionQueue {
    std::deque<std::function<void()>> deletors;

    void push_function(std::function<void()>&& function) {
        deletors.push_back(function);
    }

    void flush() {
        // reverse iterate the deletion queue to execute all the functions
        for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
            (*it)(); //call the function
        }

        deletors.clear();
    }
};

class ViEngine {
public:

    bool isInitialized{ false };
    int frameNumber { 0 };

    VkExtent2D windowExtent{ 1700 , 900 };

    GLFWwindow* window{ nullptr };
    
    //initializes everything in the engine
    void init();

    //shuts down the engine
    void cleanup();

    //draw loop
    void draw();

    //run main loop
    void run();

    VkInstance instance; // Vulkan library handle
    VkDebugUtilsMessengerEXT debugMessenger; // Vulkan debug output handle
    VkPhysicalDevice chosenGPU; // GPU chosen as the default device
    VkDevice device; // Vulkan device for commands
    VkSurfaceKHR surface; // Vulkan window surface

    VkSwapchainKHR swapchain; // from other articles

    // image format expected by the windowing system
    VkFormat swapchainImageFormat;

    //array of images from the swapchain
    std::vector<VkImage> swapchainImages;

    //array of image-views from the swapchain
    std::vector<VkImageView> swapchainImageViews;

    VkQueue graphicsQueue; //queue we will submit to
    uint32_t graphicsQueueFamily; //family of that queue

    VkCommandPool commandPool; //the command pool for our commands
    VkCommandBuffer mainCommandBuffer; //the buffer we will record into

    VkRenderPass renderPass;

    std::vector<VkFramebuffer> framebuffers;

    VkSemaphore presentSemaphore, renderSemaphore;
    VkFence renderFence;

    VkPipelineLayout trianglePipelineLayout;
    VkPipeline trianglePipeline;
    VkPipeline redTrianglePipeline;

private:
    void initVulkan();
    void initSwapchain();
    void initCommands();
    void initDefaultRenderpass();
    void initFramebuffers();
    void initSyncStructures();
    void initPipelines();

    //loads a shader module from a spir-v file. Returns false if it errors
    bool loadShaderModule(const char* filePath, VkShaderModule* outShaderModule);

    void loadMeshes();

    void uploadMesh(Mesh& mesh);

    DeletionQueue mainDeletionQueue;
    VmaAllocator allocator;

    VkPipeline meshPipeline;
    Mesh triangleMesh;
};

class PipelineBuilder {
public:
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    VkPipelineInputAssemblyStateCreateInfo inputAssembly;
    VkViewport viewport;
    VkRect2D scissor;
    VkPipelineRasterizationStateCreateInfo rasterizer;
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    VkPipelineMultisampleStateCreateInfo multisampling;
    VkPipelineLayout pipelineLayout;

    VkPipeline buildPipeline(VkDevice device, VkRenderPass pass);
};
