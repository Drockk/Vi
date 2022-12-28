#include "ViEngine.hpp"

#include "ViTypes.hpp"
#include "ViInit.hpp"

#include <fstream>

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

	//Loop until the user closes the window
	isInitialized = true;
}

void ViEngine::cleanup() {
	if (isInitialized) {
		//make sure the GPU has stopped doing its things
		vkWaitForFences(device, 1, &renderFence, true, 1000000000);

		mainDeletionQueue.flush();

		vkDestroyDevice(device, nullptr);
		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkb::destroy_debug_utils_messenger(instance, debugMessenger);
		vkDestroyInstance(instance, nullptr);

		glfwDestroyWindow(window);
		glfwTerminate();
	}
}

void ViEngine::draw() {
	//wait until the GPU has finished rendering the last frame. Timeout of 1 second
	VK_CHECK(vkWaitForFences(device, 1, &renderFence, true, 1000000000));
	VK_CHECK(vkResetFences(device, 1, &renderFence));

	//request image from the swapchain, one second timeout
	uint32_t swapchainImageIndex;
	VK_CHECK(vkAcquireNextImageKHR(device, swapchain, 1000000000, presentSemaphore, nullptr, &swapchainImageIndex));

	//now that we are sure that the commands finished executing, we can safely reset the command buffer to begin recording again.
	VK_CHECK(vkResetCommandBuffer(mainCommandBuffer, 0));

	//naming it cmd for shorter writing
	VkCommandBuffer cmd = mainCommandBuffer;

	//begin the command buffer recording. We will use this command buffer exactly once, so we want to let Vulkan know that
	VkCommandBufferBeginInfo cmdBeginInfo = {};
	cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBeginInfo.pNext = nullptr;

	cmdBeginInfo.pInheritanceInfo = nullptr;
	cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	//make a clear-color from frame number. This will flash with a 120*pi frame period.
	VkClearValue clearValue;
	float flash = abs(sin(frameNumber / 120.f));
	clearValue.color = { { 0.0f, 0.0f, flash, 1.0f } };

	//start the main renderpass.
	//We will use the clear color from above, and the framebuffer of the index the swapchain gave us
	VkRenderPassBeginInfo rpInfo = {};
	rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpInfo.pNext = nullptr;

	rpInfo.renderPass = renderPass;
	rpInfo.renderArea.offset.x = 0;
	rpInfo.renderArea.offset.y = 0;
	rpInfo.renderArea.extent = windowExtent;
	rpInfo.framebuffer = framebuffers[swapchainImageIndex];

	//connect clear values
	rpInfo.clearValueCount = 1;
	rpInfo.pClearValues = &clearValue;

	vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

	if (selectedShader == 0)
	{
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, trianglePipeline);
	}
	else
	{
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, redTrianglePipeline);
	}


	vkCmdDraw(cmd, 3, 1, 0, 0);


	//finalize the render pass
	vkCmdEndRenderPass(cmd);
	//finalize the command buffer (we can no longer add commands, but it can now be executed)
	VK_CHECK(vkEndCommandBuffer(cmd));

	//prepare the submission to the queue.
	//we want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
	//we will signal the _renderSemaphore, to signal that rendering has finished

	VkSubmitInfo submit = {};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.pNext = nullptr;

	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	submit.pWaitDstStageMask = &waitStage;

	submit.waitSemaphoreCount = 1;
	submit.pWaitSemaphores = &presentSemaphore;

	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores = &renderSemaphore;

	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &cmd;

	//submit command buffer to the queue and execute it.
	// _renderFence will now block until the graphic commands finish execution
	VK_CHECK(vkQueueSubmit(graphicsQueue, 1, &submit, renderFence));

	// this will put the image we just rendered into the visible window.
	// we want to wait on the _renderSemaphore for that,
	// as it's necessary that drawing commands have finished before the image is displayed to the user
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;

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
}

void ViEngine::initSwapchain() {
	vkb::SwapchainBuilder swapchainBuilder{ chosenGPU,device,surface };

	vkb::Swapchain vkbSwapchain = swapchainBuilder.use_default_format_selection().set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR).set_desired_extent(windowExtent.width, windowExtent.height).build().value();

	//store swapchain and its related images
	swapchain = vkbSwapchain.swapchain;
	swapchainImages = vkbSwapchain.get_images().value();
	swapchainImageViews = vkbSwapchain.get_image_views().value();

	swapchainImageFormat = vkbSwapchain.image_format;

	mainDeletionQueue.push_function([=]() {
		vkDestroySwapchainKHR(device, swapchain, nullptr);
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

	//we are going to create 1 subpass, which is the minimum you can do
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;

	VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

	//connect the color attachment to the info
	render_pass_info.attachmentCount = 1;
	render_pass_info.pAttachments = &color_attachment;
	//connect the subpass to the info
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;


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

		fb_info.pAttachments = &swapchainImageViews[i];
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
		std::cout << "Triangle fragment shader successfully loaded" << std::endl;
	}

	VkShaderModule triangleVertexShader;
	if (!loadShaderModule("../shaders/triangle.vert.spv", &triangleVertexShader))
	{
		std::cout << "Error when building the triangle vertex shader module" << std::endl;

	}
	else {
		std::cout << "Triangle vertex shader successfully loaded" << std::endl;
	}

	//compile red triangle modules
	VkShaderModule redTriangleFragShader;
	if (!loadShaderModule("../shaders/redTriangle.frag.spv", &redTriangleFragShader))
	{
		std::cout << "Error when building the triangle fragment shader module" << std::endl;
	}
	else {
		std::cout << "Red Triangle fragment shader successfully loaded" << std::endl;
	}

	VkShaderModule redTriangleVertShader;
	if (!loadShaderModule("../shaders/redTriangle.vert.spv", &redTriangleVertShader))
	{
		std::cout << "Error when building the triangle vertex shader module" << std::endl;
	}
	else {
		std::cout << "Red Triangle vertex shader successfully loaded" << std::endl;
	}

	//build the pipeline layout that controls the inputs/outputs of the shader
	//we are not using descriptor sets or other systems yet, so no need to use anything other than empty default
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = viinit::pipelineLayoutCreateInfo();

	VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &trianglePipelineLayout));

	//build the stage-create-info for both vertex and fragment stages. This lets the pipeline know the shader modules per stage
	PipelineBuilder pipelineBuilder;

	pipelineBuilder.shaderStages.push_back(
		viinit::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, triangleVertexShader));

	pipelineBuilder.shaderStages.push_back(
		viinit::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, triangleFragShader));


	//vertex input controls how to read vertices from vertex buffers. We aren't using it yet
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

	//we don't use multisampling, so just run the default one
	pipelineBuilder.multisampling = viinit::multisamplingStateCreateInfo();

	//a single blend attachment with no blending and writing to RGBA
	pipelineBuilder.colorBlendAttachment = viinit::colorBlendAttachmentState();

	//use the triangle layout we created
	pipelineBuilder.pipelineLayout = trianglePipelineLayout;

	//finally build the pipeline
	trianglePipeline = pipelineBuilder.buildPipeline(device, renderPass);

	//clear the shader stages for the builder
	pipelineBuilder.shaderStages.clear();

	//add the other shaders
	pipelineBuilder.shaderStages.push_back(
		viinit::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, redTriangleVertShader));

	pipelineBuilder.shaderStages.push_back(
		viinit::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, redTriangleFragShader));

	//build the red triangle pipeline
	redTrianglePipeline = pipelineBuilder.buildPipeline(device, renderPass);

	vkDestroyShaderModule(device, redTriangleVertShader, nullptr);
	vkDestroyShaderModule(device, redTriangleFragShader, nullptr);
	vkDestroyShaderModule(device, triangleFragShader, nullptr);
	vkDestroyShaderModule(device, triangleVertexShader, nullptr);

	mainDeletionQueue.push_function([=]() {
		//destroy the 2 pipelines we have created
		vkDestroyPipeline(device, redTrianglePipeline, nullptr);
	vkDestroyPipeline(device, trianglePipeline, nullptr);

	//destroy the pipeline layout that they use
	vkDestroyPipelineLayout(device, trianglePipelineLayout, nullptr);
		});
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

VkPipeline PipelineBuilder::buildPipeline(VkDevice device, VkRenderPass pass) {
	//make viewport state from our stored viewport and scissor.
	//at the moment we won't support multiple viewports or scissors
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.pNext = nullptr;

	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	//setup dummy color blending. We aren't using transparent objects yet
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
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = pass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	//it's easy to error out on create graphics pipeline, so we handle it a bit better than the common VK_CHECK case
	VkPipeline newPipeline;
	if (vkCreateGraphicsPipelines(
		device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline) != VK_SUCCESS) {
		std::cout << "failed to create pipeline\n";
		return VK_NULL_HANDLE; // failed to create graphics pipeline
	}
	else
	{
		return newPipeline;
	}
}
