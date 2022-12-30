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

//add the include for glm to get matrices
#include <glm/glm.hpp>

struct MeshPushConstants {
    glm::vec4 data;
    glm::mat4 render_matrix;
};

struct Material {
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
};

struct RenderObject {
    Mesh* mesh;

    Material* material;

    glm::mat4 transformMatrix;
};

struct FrameData {
    VkSemaphore presentSemaphore, renderSemaphore;
    VkFence renderFence;

    VkCommandPool commandPool;
    VkCommandBuffer mainCommandBuffer;
};

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

//number of frames to overlap when rendering
constexpr unsigned int FRAME_OVERLAP = 2;

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

    VkRenderPass renderPass;

    std::vector<VkFramebuffer> framebuffers;

private:
    void initVulkan();
    void initSwapchain();
    void initCommands();
    void initDefaultRenderpass();
    void initFramebuffers();
    void initSyncStructures();
    void initPipelines();
    void initScene();

    //loads a shader module from a spir-v file. Returns false if it errors
    bool loadShaderModule(const char* filePath, VkShaderModule* outShaderModule);

    void loadMeshes();

    void uploadMesh(Mesh& mesh);

    DeletionQueue mainDeletionQueue;
    VmaAllocator allocator;

    VkPipeline meshPipeline;
    VkPipelineLayout meshPipelineLayout;

    VkImageView depthImageView;
    AllocatedImage depthImage;
    VkFormat depthFormat;

    //default array of renderable objects
    std::vector<RenderObject> renderables;

    std::unordered_map<std::string, Material> materials;
    std::unordered_map<std::string, Mesh> meshes;
    //functions

    //create material and add it to the map
    Material* createMaterial(VkPipeline pipeline, VkPipelineLayout layout, const std::string& name);

    //returns nullptr if it can't be found
    Material* getMaterial(const std::string& name);

    //returns nullptr if it can't be found
    Mesh* getMesh(const std::string& name);

    //our draw function
    void drawObjects(VkCommandBuffer cmd, RenderObject* first, int count);

    //frame storage
    FrameData frames[FRAME_OVERLAP];

    //getter for the frame we are rendering to right now.
    FrameData& getCurrentFrame();
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
    //others
    VkPipelineDepthStencilStateCreateInfo depthStencil;

    VkPipeline buildPipeline(VkDevice device, VkRenderPass pass);
};
