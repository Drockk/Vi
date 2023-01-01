// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <ViTypes.hpp>
#include <vector>
#include <functional>
#include <deque>
#include <ViMesh.hpp>
#include <unordered_map>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

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
	VkPipelineDepthStencilStateCreateInfo depthStencil;
	VkPipeline buildPipeline(VkDevice device, VkRenderPass pass);
};



struct DeletionQueue
{
	std::deque<std::function<void()>> deletors;

	void push_function(std::function<void()>&& function) {
		deletors.push_back(function);
	}

	void flush() {
		// reverse iterate the deletion queue to execute all the functions
		for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
			(*it)(); //call functors
		}

		deletors.clear();
	}
};

struct MeshPushConstants {
	glm::vec4 data;
	glm::mat4 render_matrix;
};


struct Material {
	VkDescriptorSet textureSet{ VK_NULL_HANDLE };
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
};

struct Texture {
	AllocatedImage image;
	VkImageView imageView;
};

struct RenderObject {
	Mesh* mesh;

	Material* material;

	glm::mat4 transformMatrix;


};


struct FrameData {
	VkSemaphore presentSemaphore, renderSemaphore;
	VkFence renderFence;

	DeletionQueue frameDeletionQueue;

	VkCommandPool commandPool;
	VkCommandBuffer mainCommandBuffer;

	AllocatedBuffer cameraBuffer;
	VkDescriptorSet globalDescriptor;

	AllocatedBuffer objectBuffer;
	VkDescriptorSet objectDescriptor;
};

struct UploadContext {
	VkFence uploadFence;
	VkCommandPool commandPool;
	VkCommandBuffer commandBuffer;
};
struct GPUCameraData {
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 viewproj;
};


struct GPUSceneData {
	glm::vec4 fogColor; // w is for exponent
	glm::vec4 fogDistances; //x for min, y for max, zw unused.
	glm::vec4 ambientColor;
	glm::vec4 sunlightDirection; //w for sun power
	glm::vec4 sunlightColor;
};

struct GPUObjectData {
	glm::mat4 modelMatrix;
};

constexpr unsigned int FRAME_OVERLAP = 2;

class ViEngine {
public:

	bool isInitialized{ false };
	int frameNumber{ 0 };
	int selectedShader{ 0 };

	VkExtent2D windowExtent{ 1700 , 900 };

	GLFWwindow* window{ nullptr };

	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkPhysicalDevice chosenGPU;
	VkDevice device;

	VkPhysicalDeviceProperties gpuProperties;

	FrameData frames[FRAME_OVERLAP];

	VkQueue graphicsQueue;
	uint32_t graphicsQueueFamily;

	VkRenderPass renderPass;

	VkSurfaceKHR surface;
	VkSwapchainKHR swapchain;
	VkFormat swapchainImageFormat;

	std::vector<VkFramebuffer> framebuffers;
	std::vector<VkImage> swapchainImages;
	std::vector<VkImageView> swapchainImageViews;

	DeletionQueue mainDeletionQueue;

	VmaAllocator allocator; //vma lib allocator

	//depth resources
	VkImageView depthImageView;
	AllocatedImage depthImage;

	//the format for the depth image
	VkFormat depthFormat;

	VkDescriptorPool descriptorPool;

	VkDescriptorSetLayout globalSetLayout;
	VkDescriptorSetLayout objectSetLayout;
	VkDescriptorSetLayout singleTextureSetLayout;

	GPUSceneData sceneParameters;
	AllocatedBuffer sceneParameterBuffer;

	UploadContext uploadContext;
	//initializes everything in the engine
	void init();

	//shuts down the engine
	void cleanup();

	//draw loop
	void draw();

	//run main loop
	void run();

	FrameData& getCurrentFrame();
	FrameData& getLastFrame();

	//default array of renderable objects
	std::vector<RenderObject> renderables;

	std::unordered_map<std::string, Material> materials;
	std::unordered_map<std::string, Mesh> meshes;
	std::unordered_map<std::string, Texture> loadedTextures;
	//functions

	//create material and add it to the map
	Material* createMaterial(VkPipeline pipeline, VkPipelineLayout layout, const std::string& name);

	//returns nullptr if it cant be found
	Material* getMaterial(const std::string& name);

	//returns nullptr if it cant be found
	Mesh* getMesh(const std::string& name);

	//our draw function
	void drawObjects(VkCommandBuffer cmd, RenderObject* first, int count);

	AllocatedBuffer createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);

	size_t padUniformBufferSize(size_t originalSize);

	void immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);
private:

	void initVulkan();

	void initSwapchain();

	void initDefaultRenderpass();

	void initFramebuffers();

	void initCommands();

	void initSyncStructures();

	void initPipelines();

	void initScene();

	void initDescriptors();

	//loads a shader module from a spir-v file. Returns false if it errors
	bool loadShaderModule(const char* filePath, VkShaderModule* outShaderModule);

	void loadMeshes();

	void loadImages();

	void uploadMesh(Mesh& mesh);
};