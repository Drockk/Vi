#include "ViEngine.hpp"

#include "ViTypes.hpp"
#include "ViInit.hpp"

#include <fstream>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include <glm/gtx/transform.hpp>

static int selectedShader{ 0 };

void ViEngine::init() {
	//Initialize the library
	if (!glfwInit()) {
		
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(windowExtent.width, windowExtent.height, "Vi", nullptr, nullptr);
	if (!window)
	{
		glfwTerminate();
	}

	//Make the window's context current
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
		if(key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
			selectedShader += 1;
			if (selectedShader > 1)
			{
				selectedShader = 0;
			}
		}
	});

	initVulkan();
	initSwapchain();
	initCommands();
	initDefaultRenderpass();
	initFramebuffers();
	initSyncStructures();
	initPipelines();

	loadMeshes();

	initScene();

	//Loop until the user closes the window
	isInitialized = true;
}

void ViEngine::cleanup() {
	if (isInitialized) {
		//make sure the GPU has stopped doing its things
		vkWaitForFences(device, 1, &renderFence, true, 1000000000);

		mainDeletionQueue.flush();

		vmaDestroyAllocator(allocator);
		vkDestroyDevice(device, nullptr);
		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkb::destroy_debug_utils_messenger(instance, debugMessenger);
		vkDestroyInstance(instance, nullptr);

		glfwDestroyWindow(window);
		glfwTerminate();
	}
}

void ViEngine::draw() {
	//wait until the gpu has finished rendering the last frame. Timeout of 1 second
	VK_CHECK(vkWaitForFences(device, 1, &renderFence, true, 1000000000));
	VK_CHECK(vkResetFences(device, 1, &renderFence));

	//now that we are sure that the commands finished executing, we can safely reset the command buffer to begin recording again.
	VK_CHECK(vkResetCommandBuffer(mainCommandBuffer, 0));

	//request image from the swapchain
	uint32_t swapchainImageIndex;
	VK_CHECK(vkAcquireNextImageKHR(device, swapchain, 1000000000, presentSemaphore, nullptr, &swapchainImageIndex));

	//naming it cmd for shorter writing
	VkCommandBuffer cmd = mainCommandBuffer;

	//begin the command buffer recording. We will use this command buffer exactly once, so we want to let vulkan know that
	VkCommandBufferBeginInfo cmdBeginInfo = viinit::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	//make a clear-color from frame number. This will flash with a 120 frame period.
	VkClearValue clearValue;
	float flash = abs(sin(frameNumber / 120.f));
	clearValue.color = { { 0.0f, 0.0f, flash, 1.0f } };

	//clear depth at 1
	VkClearValue depthClear;
	depthClear.depthStencil.depth = 1.f;

	//start the main renderpass. 
	//We will use the clear color from above, and the framebuffer of the index the swapchain gave us
	VkRenderPassBeginInfo rpInfo = viinit::renderpassBeginInfo(renderPass, windowExtent, framebuffers[swapchainImageIndex]);

	//connect clear values
	rpInfo.clearValueCount = 2;

	VkClearValue clearValues[] = { clearValue, depthClear };

	rpInfo.pClearValues = &clearValues[0];

	vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

	drawObjects(cmd, renderables.data(), renderables.size());

	//finalize the render pass
	vkCmdEndRenderPass(cmd);
	//finalize the command buffer (we can no longer add commands, but it can now be executed)
	VK_CHECK(vkEndCommandBuffer(cmd));

	//prepare the submission to the queue. 
	//we want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
	//we will signal the _renderSemaphore, to signal that rendering has finished

	VkSubmitInfo submit = viinit::submitInfo(&cmd);
	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	submit.pWaitDstStageMask = &waitStage;

	submit.waitSemaphoreCount = 1;
	submit.pWaitSemaphores = &presentSemaphore;

	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores = &renderSemaphore;

	//submit command buffer to the queue and execute it.
	// _renderFence will now block until the graphic commands finish execution
	VK_CHECK(vkQueueSubmit(graphicsQueue, 1, &submit, renderFence));

	//prepare present
	// this will put the image we just rendered to into the visible window.
	// we want to wait on the _renderSemaphore for that, 
	// as its necessary that drawing commands have finished before the image is displayed to the user
	VkPresentInfoKHR presentInfo = viinit::presentInfo();

	presentInfo.pSwapchains = &swapchain;
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores = &renderSemaphore;
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices = &swapchainImageIndex;

	VK_CHECK(vkQueuePresentKHR(graphicsQueue, &presentInfo));

	//increase the number of frames drawn
	frameNumber++;
}

void ViEngine::run() {
	while(!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		draw();
	}
}

void ViEngine::initVulkan() {
	vkb::InstanceBuilder builder;

	//make the Vulkan instance, with basic debug features
	auto instRet = builder.set_app_name("Vi").request_validation_layers(true).require_api_version(1, 1, 0).use_default_debug_messenger().build();

	vkb::Instance vkbInst = instRet.value();

	//store the instance
	instance = vkbInst.instance;
	//store the debug messenger
	debugMessenger = vkbInst.debug_messenger;

	// get the surface of the window we opened with SDL
	glfwCreateWindowSurface(instance, window, nullptr, &surface);

	//use vkbootstrap to select a GPU.
	//We want a GPU that can write to the SDL surface and supports Vulkan 1.1
	vkb::PhysicalDeviceSelector selector{ vkbInst };
	auto phys_ret = selector.set_surface(surface)
		.set_minimum_version(1, 1) // require a vulkan 1.1 capable device
		.require_dedicated_transfer_queue()
		.select();
	if (!phys_ret) {
		std::cerr << "Failed to select Vulkan Physical Device. Error: " << phys_ret.error().message() << "\n";
	}

	//create the final Vulkan device
	vkb::DeviceBuilder deviceBuilder{ phys_ret.value() };

	vkb::Device vkbDevice = deviceBuilder.build().value();

	// Get the VkDevice handle used in the rest of a Vulkan application
	device = vkbDevice.device;
	chosenGPU = phys_ret.value().physical_device;

	// use vkbootstrap to get a Graphics queue
	graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
	graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

	//initialize the memory allocator
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = chosenGPU;
	allocatorInfo.device = device;
	allocatorInfo.instance = instance;
	vmaCreateAllocator(&allocatorInfo, &allocator);
}

void ViEngine::initSwapchain() {
	vkb::SwapchainBuilder swapchainBuilder{ chosenGPU,device,surface };

	vkb::Swapchain vkbSwapchain = swapchainBuilder
		.use_default_format_selection()
		//use vsync present mode
		.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
		.set_desired_extent(windowExtent.width, windowExtent.height)
		.build()
		.value();

	//store swapchain and its related images
	swapchain = vkbSwapchain.swapchain;
	swapchainImages = vkbSwapchain.get_images().value();
	swapchainImageViews = vkbSwapchain.get_image_views().value();

	swapchainImageFormat = vkbSwapchain.image_format;

	mainDeletionQueue.push_function([=]() {
		vkDestroySwapchainKHR(device, swapchain, nullptr);
		});

	//depth image size will match the window
	VkExtent3D depthImageExtent = {
		windowExtent.width,
		windowExtent.height,
		1
	};

	//hardcoding the depth format to 32 bit float
	depthFormat = VK_FORMAT_D32_SFLOAT;

	//the depth image will be a image with the format we selected and Depth Attachment usage flag
	VkImageCreateInfo dimg_info = viinit::imageCreateInfo(depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthImageExtent);

	//for the depth image, we want to allocate it from gpu local memory
	VmaAllocationCreateInfo dimg_allocinfo = {};
	dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	//allocate and create the image
	vmaCreateImage(allocator, &dimg_info, &dimg_allocinfo, &depthImage.image, &depthImage.allocation, nullptr);

	//build a image-view for the depth image to use for rendering
	VkImageViewCreateInfo dview_info = viinit::imageviewCreateInfo(depthFormat, depthImage.image, VK_IMAGE_ASPECT_DEPTH_BIT);

	VK_CHECK(vkCreateImageView(device, &dview_info, nullptr, &depthImageView));

	//add to deletion queues
	mainDeletionQueue.push_function([=]() {
		vkDestroyImageView(device, depthImageView, nullptr);
	vmaDestroyImage(allocator, depthImage.image, depthImage.allocation);
		});
}

void ViEngine::initCommands() {
	//create a command pool for commands submitted to the graphics queue.
	//we also want the pool to allow for resetting of individual command buffers
	VkCommandPoolCreateInfo commandPoolInfo = viinit::commandPoolCreateInfo(graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	VK_CHECK(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool));

	//allocate the default command buffer that we will use for rendering
	VkCommandBufferAllocateInfo cmdAllocInfo = viinit::commandBufferAllocateInfo(commandPool, 1);

	VK_CHECK(vkAllocateCommandBuffers(device, &cmdAllocInfo, &mainCommandBuffer));

	mainDeletionQueue.push_function([=]() {
		vkDestroyCommandPool(device, commandPool, nullptr);
		});
}

void ViEngine::initDefaultRenderpass() {
	// the renderpass will use this color attachment.
	VkAttachmentDescription color_attachment = {};
	//the attachment will have the format needed by the swapchain
	color_attachment.format = swapchainImageFormat;
	//1 sample, we won't be doing MSAA
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	// we Clear when this attachment is loaded
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	// we keep the attachment stored when the renderpass ends
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	//we don't care about stencil
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	//we don't know or care about the starting layout of the attachment
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	//after the renderpass ends, the image has to be on a layout ready for display
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref = {};
	//attachment number will index into the pAttachments array in the parent renderpass itself
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depth_attachment = {};
	// Depth attachment
	depth_attachment.flags = 0;
	depth_attachment.format = depthFormat;
	depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_attachment_ref = {};
	depth_attachment_ref.attachment = 1;
	depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	//we are going to create 1 subpass, which is the minimum you can do
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;
	//hook the depth attachment into the subpass
	subpass.pDepthStencilAttachment = &depth_attachment_ref;

	//array of 2 attachments, one for the color, and other for depth
	VkAttachmentDescription attachments[2] = { color_attachment,depth_attachment };

	VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	//2 attachments from said array
	render_pass_info.attachmentCount = 2;
	render_pass_info.pAttachments = &attachments[0];
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkSubpassDependency depth_dependency = {};
	depth_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	depth_dependency.dstSubpass = 0;
	depth_dependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	depth_dependency.srcAccessMask = 0;
	depth_dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	depth_dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	VkSubpassDependency dependencies[2] = { dependency, depth_dependency };

	render_pass_info.dependencyCount = 2;
	render_pass_info.pDependencies = &dependencies[0];

	VK_CHECK(vkCreateRenderPass(device, &render_pass_info, nullptr, &renderPass));

	mainDeletionQueue.push_function([=]() {
		vkDestroyRenderPass(device, renderPass, nullptr);
		});
}

void ViEngine::initFramebuffers() {
	//create the framebuffers for the swapchain images. This will connect the render-pass to the images for rendering
	VkFramebufferCreateInfo fb_info = {};
	fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fb_info.pNext = nullptr;

	fb_info.renderPass = renderPass;
	fb_info.attachmentCount = 1;
	fb_info.width = windowExtent.width;
	fb_info.height = windowExtent.height;
	fb_info.layers = 1;

	//grab how many images we have in the swapchain
	const uint32_t swapchain_imagecount = swapchainImages.size();
	framebuffers = std::vector<VkFramebuffer>(swapchain_imagecount);

	//create framebuffers for each of the swapchain image views
	for (int i = 0; i < swapchain_imagecount; i++) {

		VkImageView attachments[2];
		attachments[0] = swapchainImageViews[i];
		attachments[1] = depthImageView;

		fb_info.pAttachments = attachments;
		fb_info.attachmentCount = 2;

		VK_CHECK(vkCreateFramebuffer(device, &fb_info, nullptr, &framebuffers[i]));

		mainDeletionQueue.push_function([=]() {
			vkDestroyFramebuffer(device, framebuffers[i], nullptr);
		vkDestroyImageView(device, swapchainImageViews[i], nullptr);
			});
	}
}

void ViEngine::initSyncStructures() {
	VkFenceCreateInfo fenceCreateInfo = viinit::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);

	VK_CHECK(vkCreateFence(device, &fenceCreateInfo, nullptr, &renderFence));

	//enqueue the destruction of the fence
	mainDeletionQueue.push_function([=]() {
		vkDestroyFence(device, renderFence, nullptr);
		});

	VkSemaphoreCreateInfo semaphoreCreateInfo = viinit::semaphoreCreateInfo();

	VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &presentSemaphore));
	VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &renderSemaphore));

	//enqueue the destruction of semaphores
	mainDeletionQueue.push_function([=]() {
		vkDestroySemaphore(device, presentSemaphore, nullptr);
	vkDestroySemaphore(device, renderSemaphore, nullptr);
		});
}

void ViEngine::initPipelines() {
	VkShaderModule triangleFragShader;
	if (!loadShaderModule("../shaders/triangle.frag.spv", &triangleFragShader))
	{
		std::cout << "Error when building the triangle fragment shader module" << std::endl;
	}
	else {
		std::cout << "Triangle fragment shader succesfully loaded" << std::endl;
	}

	//build the stage-create-info for both vertex and fragment stages. This lets the pipeline know the shader modules per stage
	PipelineBuilder pipelineBuilder;
	//vertex input controls how to read vertices from vertex buffers. We arent using it yet
	pipelineBuilder.vertexInputInfo = viinit::vertexInputStateCreateInfo();

	//input assembly is the configuration for drawing triangle lists, strips, or individual points.
	//we are just going to draw triangle list
	pipelineBuilder.inputAssembly = viinit::inputAssemblyCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

	//build viewport and scissor from the swapchain extents
	pipelineBuilder.viewport.x = 0.0f;
	pipelineBuilder.viewport.y = 0.0f;
	pipelineBuilder.viewport.width = (float)windowExtent.width;
	pipelineBuilder.viewport.height = (float)windowExtent.height;
	pipelineBuilder.viewport.minDepth = 0.0f;
	pipelineBuilder.viewport.maxDepth = 1.0f;

	pipelineBuilder.scissor.offset = { 0, 0 };
	pipelineBuilder.scissor.extent = windowExtent;

	//configure the rasterizer to draw filled triangles
	pipelineBuilder.rasterizer = viinit::rasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);

	//we dont use multisampling, so just run the default one
	pipelineBuilder.multisampling = viinit::multisamplingStateCreateInfo();

	//a single blend attachment with no blending and writing to RGBA
	pipelineBuilder.colorBlendAttachment = viinit::colorBlendAttachmentState();

	//default depthtesting
	pipelineBuilder.depthStencil = viinit::depthStencilCreateInfo(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

	//build the mesh pipeline

	VertexInputDescription vertexDescription = Vertex::getVertexDescription();

	//connect the pipeline builder vertex input info to the one we get from Vertex
	pipelineBuilder.vertexInputInfo.pVertexAttributeDescriptions = vertexDescription.attributes.data();
	pipelineBuilder.vertexInputInfo.vertexAttributeDescriptionCount = vertexDescription.attributes.size();

	pipelineBuilder.vertexInputInfo.pVertexBindingDescriptions = vertexDescription.bindings.data();
	pipelineBuilder.vertexInputInfo.vertexBindingDescriptionCount = vertexDescription.bindings.size();

	//compile mesh vertex shader
	VkShaderModule meshVertShader;
	if (!loadShaderModule("../shaders/tri_mesh.vert.spv", &meshVertShader))
	{
		std::cout << "Error when building the triangle vertex shader module" << std::endl;
	}
	else {
		std::cout << "Red Triangle vertex shader succesfully loaded" << std::endl;
	}

	//add the other shaders
	pipelineBuilder.shaderStages.push_back(viinit::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, meshVertShader));

	//make sure that triangleFragShader is holding the compiled colored_triangle.frag
	pipelineBuilder.shaderStages.push_back(viinit::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, triangleFragShader));

	//we start from just the default empty pipeline layout info
	VkPipelineLayoutCreateInfo mesh_pipeline_layout_info = viinit::pipelineLayoutCreateInfo();

	//setup push constants
	VkPushConstantRange push_constant;
	//offset 0
	push_constant.offset = 0;
	//size of a MeshPushConstant struct
	push_constant.size = sizeof(MeshPushConstants);
	//for the vertex shader
	push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	mesh_pipeline_layout_info.pPushConstantRanges = &push_constant;
	mesh_pipeline_layout_info.pushConstantRangeCount = 1;

	pipelineBuilder.depthStencil = viinit::depthStencilCreateInfo(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

	VK_CHECK(vkCreatePipelineLayout(device, &mesh_pipeline_layout_info, nullptr, &meshPipelineLayout));

	//hook the push constants layout
	pipelineBuilder.pipelineLayout = meshPipelineLayout;
	//build the mesh triangle pipeline
	meshPipeline = pipelineBuilder.buildPipeline(device, renderPass);

	createMaterial(meshPipeline, meshPipelineLayout, "defaultmesh");

	vkDestroyShaderModule(device, meshVertShader, nullptr);
	vkDestroyShaderModule(device, triangleFragShader, nullptr);

	mainDeletionQueue.push_function([=]() {
		vkDestroyPipeline(device, meshPipeline, nullptr);
		vkDestroyPipelineLayout(device, meshPipelineLayout, nullptr);
	});
}

void ViEngine::initScene() {
	RenderObject monkey;
	monkey.mesh = getMesh("monkey");
	monkey.material = getMaterial("defaultmesh");
	monkey.transformMatrix = glm::mat4{ 1.0f };

	renderables.push_back(monkey);

	for (int x = -20; x <= 20; x++) {
		for (int y = -20; y <= 20; y++) {

			RenderObject tri;
			tri.mesh = getMesh("triangle");
			tri.material = getMaterial("defaultmesh");
			glm::mat4 translation = glm::translate(glm::mat4{ 1.0 }, glm::vec3(x, 0, y));
			glm::mat4 scale = glm::scale(glm::mat4{ 1.0 }, glm::vec3(0.2, 0.2, 0.2));
			tri.transformMatrix = translation * scale;

			renderables.push_back(tri);
		}
	}
}

bool ViEngine::loadShaderModule(const char* filePath, VkShaderModule* outShaderModule) {
	//open the file. With cursor at the end
	std::ifstream file(filePath, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		return false;
	}

	//find what the size of the file is by looking up the location of the cursor
	//because the cursor is at the end, it gives the size directly in bytes
	size_t fileSize = file.tellg();

	//spirv expects the buffer to be on uint32, so make sure to reserve an int vector big enough for the entire file
	std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

	//put file cursor at beginning
	file.seekg(0);

	//load the entire file into the buffer
	file.read((char*)buffer.data(), fileSize);

	//now that the file is loaded into the buffer, we can close it
	file.close();

	//create a new shader module, using the buffer we loaded
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.pNext = nullptr;

	//codeSize has to be in bytes, so multiply the ints in the buffer by size of int to know the real size of the buffer
	createInfo.codeSize = buffer.size() * sizeof(uint32_t);
	createInfo.pCode = buffer.data();

	//check that the creation goes well.
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		return false;
	}
	*outShaderModule = shaderModule;
	return true;
}

void ViEngine::loadMeshes() {
	Mesh triangleMesh;
	triangleMesh.vertices.resize(3);

	triangleMesh.vertices[0].position = { 1.f,1.f, 0.5f };
	triangleMesh.vertices[1].position = { -1.f,1.f, 0.5f };
	triangleMesh.vertices[2].position = { 0.f,-1.f, 0.5f };

	triangleMesh.vertices[0].color = { 0.f,1.f, 0.0f }; //pure green
	triangleMesh.vertices[1].color = { 0.f,1.f, 0.0f }; //pure green
	triangleMesh.vertices[2].color = { 0.f,1.f, 0.0f }; //pure green

	Mesh monkeyMesh;
	monkeyMesh.loadFromObj("../assets/monkey_smooth.obj");


	uploadMesh(triangleMesh);
	uploadMesh(monkeyMesh);

	//note that we are copying them. Eventually we will delete the hardcoded _monkey and _triangle meshes, so it's no problem now.
	meshes["monkey"] = monkeyMesh;
	meshes["triangle"] = triangleMesh;
}

void ViEngine::uploadMesh(Mesh& mesh) {
	//allocate vertex buffer
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	//this is the total size, in bytes, of the buffer we are allocating
	bufferInfo.size = mesh.vertices.size() * sizeof(Vertex);
	//this buffer is going to be used as a Vertex Buffer
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;


	//let the VMA library know that this data should be writeable by CPU, but also readable by GPU
	VmaAllocationCreateInfo vmaallocInfo = {};
	vmaallocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	//allocate the buffer
	VK_CHECK(vmaCreateBuffer(allocator, &bufferInfo, &vmaallocInfo,
		&mesh.vertexBuffer.buffer,
		&mesh.vertexBuffer.allocation,
		nullptr));

	//add the destruction of triangle mesh buffer to the deletion queue
	mainDeletionQueue.push_function([=]() {

		vmaDestroyBuffer(allocator, mesh.vertexBuffer.buffer, mesh.vertexBuffer.allocation);
		});

	//copy vertex data
	void* data;
	vmaMapMemory(allocator, mesh.vertexBuffer.allocation, &data);

	memcpy(data, mesh.vertices.data(), mesh.vertices.size() * sizeof(Vertex));

	vmaUnmapMemory(allocator, mesh.vertexBuffer.allocation);
}

Material* ViEngine::createMaterial(VkPipeline pipeline, VkPipelineLayout layout, const std::string& name) {
	Material mat;
	mat.pipeline = pipeline;
	mat.pipelineLayout = layout;
	materials[name] = mat;
	return &materials[name];
}

Material* ViEngine::getMaterial(const std::string& name) {
	//search for the object, and return nullptr if not found
	auto it = materials.find(name);
	if (it == materials.end()) {
		return nullptr;
	}
	else {
		return &(*it).second;
	}
}

Mesh* ViEngine::getMesh(const std::string& name) {
	auto it = meshes.find(name);
	if (it == meshes.end()) {
		return nullptr;
	}
	else {
		return &(*it).second;
	}
}

void ViEngine::drawObjects(VkCommandBuffer cmd, RenderObject* first, int count) {
	//make a model view matrix for rendering the object
	//camera view
	glm::vec3 camPos = { 0.f,-6.f,-10.f };

	glm::mat4 view = glm::translate(glm::mat4(1.f), camPos);
	//camera projection
	glm::mat4 projection = glm::perspective(glm::radians(70.f), 1700.f / 900.f, 0.1f, 200.0f);
	projection[1][1] *= -1;

	Mesh* lastMesh = nullptr;
	Material* lastMaterial = nullptr;
	for (int i = 0; i < count; i++)
	{
		RenderObject& object = first[i];

		//only bind the pipeline if it doesn't match with the already bound one
		if (object.material != lastMaterial) {

			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipeline);
			lastMaterial = object.material;
		}


		glm::mat4 model = object.transformMatrix;
		//final render matrix, that we are calculating on the cpu
		glm::mat4 mesh_matrix = projection * view * model;

		MeshPushConstants constants;
		constants.render_matrix = mesh_matrix;

		//upload the mesh to the GPU via push constants
		vkCmdPushConstants(cmd, object.material->pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);

		//only bind the mesh if it's a different one from last bind
		if (object.mesh != lastMesh) {
			//bind the mesh vertex buffer with offset 0
			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(cmd, 0, 1, &object.mesh->vertexBuffer.buffer, &offset);
			lastMesh = object.mesh;
		}
		//we can now draw
		vkCmdDraw(cmd, object.mesh->vertices.size(), 1, 0, 0);
	}
}

VkPipeline PipelineBuilder::buildPipeline(VkDevice device, VkRenderPass pass) {
	//make viewport state from our stored viewport and scissor.
		//at the moment we wont support multiple viewports or scissors
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.pNext = nullptr;

	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	//setup dummy color blending. We arent using transparent objects yet
	//the blending is just "no blend", but we do write to the color attachment
	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.pNext = nullptr;

	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;

	//build the actual pipeline
	//we now use all of the info structs we have been writing into into this one to create the pipeline
	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pNext = nullptr;

	pipelineInfo.stageCount = shaderStages.size();
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = pass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	//its easy to error out on create graphics pipeline, so we handle it a bit better than the common VK_CHECK case
	VkPipeline newPipeline;
	if (vkCreateGraphicsPipelines(
		device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline) != VK_SUCCESS) {
		std::cout << "failed to create pipline\n";
		return VK_NULL_HANDLE; // failed to create graphics pipeline
	}
	else
	{
		return newPipeline;
	}
}
