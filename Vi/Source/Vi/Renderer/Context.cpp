#include "vipch.hpp"
#include "Vi/Renderer/Context.hpp"

#include <fstream>

namespace Vi {
    void Context::init(const std::shared_ptr<Window>& window) {
        initInstance(window->getName());
        initSurface(window);
        initDevice();
        initSwapchain();

        getGraphicsQueue();
        getPresentQueue();

        createRenderpass();
        createGraphicsPipeline();

        createFramebuffers();
        createCommandPool();
    }

    void Context::shutdown() const {
        vkDeviceWaitIdle(m_Device);
    }

    void Context::initInstance(const std::string& name) {
        vkb::InstanceBuilder builder;
        auto instRet = builder.set_app_name(name.c_str())
            .set_engine_name("Vi Engine")
            .require_api_version(1, 1, 0)
            .request_validation_layers(true)
            .set_debug_callback([](VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, [[maybe_unused]] void* pUserData) -> VkBool32 {
                auto type = vkb::to_string_message_type(messageType);

                switch (messageSeverity) {
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                    VI_CORE_TRACE("[{0}]: {1}", type, pCallbackData->pMessage);
                    break;
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                    VI_CORE_INFO("[{0}]: {1}", type, pCallbackData->pMessage);
                    break;
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                    VI_CORE_WARN("[{0}]: {1}", type, pCallbackData->pMessage);
                    break;
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                    VI_CORE_ERROR("[{0}]: {1}", type, pCallbackData->pMessage);
                    break;
                default:
                    VI_CORE_DEBUG("[{0}]: {1}", type, pCallbackData->pMessage);
                    break;
                }

                return VK_FALSE;
            })
            .build();

        ASSERT_CORE_LOG(instRet.has_value(), "Failed to create Vulkan instance.");

        m_Instance = instRet.value();

        ShutdownQueue::addToQueue([instance = m_Instance] {
            destroy_instance(instance);
        });
    }

    void Context::initSurface(const std::shared_ptr<Window>& window) {
        window->createSurface(m_Surface, m_Instance.instance);

        ShutdownQueue::addToQueue([instance = m_Instance, surface = m_Surface] {
            destroy_surface(instance, surface);
        });
    }

    void Context::initDevice() {
        vkb::PhysicalDeviceSelector selector{ m_Instance };
        auto physRet = selector.set_surface(m_Surface)
            .set_minimum_version(1, 1)
            .require_dedicated_transfer_queue()
            .select();

        ASSERT_CORE_LOG(physRet.has_value(), "Failed to select physical device");

        const auto physicDevice = physRet.value();
        const vkb::DeviceBuilder deviceBuilder{ physicDevice };
        auto devRet = deviceBuilder.build();
        ASSERT_CORE_LOG(devRet.has_value(), "Failed to create Vulkan device");
        m_Device = devRet.value();

        ShutdownQueue::addToQueue([device = m_Device] {
            destroy_device(device);
        });
    }

    void Context::initSwapchain() {
        vkb::SwapchainBuilder swapchainBuilder{ m_Device };
        auto swapchainRet = swapchainBuilder.set_old_swapchain(m_Swapchain).build();
        ASSERT_CORE_LOG(swapchainRet.has_value(), "Failed to build swapchain");
        destroy_swapchain(m_Swapchain);

        m_Swapchain = swapchainRet.value();
        m_SwapchainImages = m_Swapchain.get_images().value();
        m_SwapchainImageViews = m_Swapchain.get_image_views().value();

        ShutdownQueue::addToQueue([device = m_Device, swapchainImageViews = m_SwapchainImageViews] {
            for(const auto imageView: swapchainImageViews) {
                vkDestroyImageView(device, imageView, nullptr);
            }
        });

        ShutdownQueue::addToQueue([swapchain = m_Swapchain] {
            destroy_swapchain(swapchain);
        });
    }

    void Context::getGraphicsQueue() {
        auto graphicsQueueRet = m_Device.get_queue(vkb::QueueType::graphics);
        ASSERT_CORE_LOG(graphicsQueueRet.has_value(), "Failed to get graphics queue");
        m_GraphicsQueue = graphicsQueueRet.value();
    }

    void Context::getPresentQueue() {
        auto presentQueueRet = m_Device.get_queue(vkb::QueueType::present);
        ASSERT_CORE_LOG(presentQueueRet.has_value(), "Failed to get present queue");
        m_PresentQueue = presentQueueRet.value();
    }

    void Context::createRenderpass() {
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = m_Swapchain.image_format;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassCreateInfo = {};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = 1;
        renderPassCreateInfo.pAttachments = &colorAttachment;
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpass;
        renderPassCreateInfo.dependencyCount = 1;
        renderPassCreateInfo.pDependencies = &dependency;

        ASSERT_CORE_LOG(vkCreateRenderPass(m_Device, &renderPassCreateInfo, nullptr, &m_RenderPass) == VK_SUCCESS, "Failed to create render pass");

        ShutdownQueue::addToQueue([device = m_Device.device, renderPass = m_RenderPass] {
            vkDestroyRenderPass(device, renderPass, nullptr);
        });
    }

    void Context::createGraphicsPipeline() {
        auto vertCode = readFile("shaders/vert.spv");
        auto fragCode = readFile("shaders/frag.spv");

        VkShaderModule vertModule = createShaderModule(vertCode);
        ASSERT_CORE_LOG(vertModule != VK_NULL_HANDLE, "Failed to create shader module");
        VkShaderModule fragModule = createShaderModule(fragCode);
        ASSERT_CORE_LOG(fragModule != VK_NULL_HANDLE, "Failed to create shader module");

        VkPipelineShaderStageCreateInfo vertStageInfo = {};
        vertStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertStageInfo.module = vertModule;
        vertStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragStageInfo = {};
        fragStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragStageInfo.module = fragModule;
        fragStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertStageInfo, fragStageInfo };

        VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;

        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_Swapchain.extent.width);
        viewport.height = static_cast<float>(m_Swapchain.extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor = {};
        scissor.offset = { 0, 0 };
        scissor.extent = m_Swapchain.extent;

        VkPipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer = {};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling = {};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending = {};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        ASSERT_CORE_LOG(vkCreatePipelineLayout(m_Device.device, &pipelineLayoutInfo, nullptr, &m_PipelineLayout) == VK_SUCCESS, "Failed to create pipeline layout");

        ShutdownQueue::addToQueue([device = m_Device.device, pipelineLayout = m_PipelineLayout] {
            vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        });

        std::vector dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

        VkPipelineDynamicStateCreateInfo dynamicInfo = {};
        dynamicInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicInfo.dynamicStateCount = static_cast<uint32_t> (dynamicStates.size());
        dynamicInfo.pDynamicStates = dynamicStates.data();

        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicInfo;
        pipelineInfo.layout = m_PipelineLayout;
        pipelineInfo.renderPass = m_RenderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        ASSERT_CORE_LOG(vkCreateGraphicsPipelines(m_Device.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline) == VK_SUCCESS, "Failed to graphics create pipeline");

        ShutdownQueue::addToQueue([device = m_Device.device, graphicsPipeline = m_GraphicsPipeline] {
            vkDestroyPipeline(device, graphicsPipeline, nullptr);
        });

        vkDestroyShaderModule(m_Device.device, fragModule, nullptr);
        vkDestroyShaderModule(m_Device.device, vertModule, nullptr);
    }

    std::vector<char> Context::readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        ASSERT_CORE_LOG(file.is_open(), "Failed to open shader file!");

        const auto fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), static_cast<std::streamsize>(fileSize));
        file.close();

        return buffer;
    }

    VkShaderModule Context::createShaderModule(const std::vector<char>& code) const {
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if(vkCreateShaderModule(m_Device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            return VK_NULL_HANDLE;
        }

        return shaderModule;
    }

    void Context::createFramebuffers() {
        m_Framebuffers.resize(m_SwapchainImageViews.size());

        for(size_t i = 0; i < m_SwapchainImageViews.size(); i++) {
            const VkImageView attachments[] = { m_SwapchainImageViews[i] };

            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_RenderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = m_Swapchain.extent.width;
            framebufferInfo.height = m_Swapchain.extent.height;
            framebufferInfo.layers = 1;

            ASSERT_CORE_LOG(vkCreateFramebuffer(m_Device, &framebufferInfo, nullptr, &m_Framebuffers[i]) == VK_SUCCESS, "Failed to create framebuffer");

            for (const auto fb : m_Framebuffers) {
                ShutdownQueue::addToQueue([device = m_Device.device, framebuffer = fb] {
                    vkDestroyFramebuffer(device, framebuffer, nullptr);
                });
            }
        }
    }

    void Context::createCommandPool() {
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = m_Device.get_queue_index(vkb::QueueType::graphics).value();

        ASSERT_CORE_LOG(vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_CommandPool) == VK_SUCCESS, "Failed to create command pool");

        ShutdownQueue::addToQueue([device = m_Device.device, commandPool = m_CommandPool] {
            vkDestroyCommandPool(device, commandPool, nullptr);
        });
    }

    void Context::createCommandBuffers() {
        m_CommandBuffers.resize(m_Framebuffers.size());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_CommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());

        ASSERT_CORE_LOG(vkAllocateCommandBuffers(m_Device, &allocInfo, m_CommandBuffers.data()) == VK_SUCCESS,"Failed to allocate command buffers");

        for (size_t i{ 0 }; i < m_CommandBuffers.size(); i++) {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            ASSERT_CORE_LOG(vkBeginCommandBuffer(m_CommandBuffers[i], &beginInfo) == VK_SUCCESS, "Failed to begin recording command buffer");

            VkRenderPassBeginInfo renderPassInfo = {};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = m_RenderPass;
            renderPassInfo.framebuffer = m_Framebuffers[i];
            renderPassInfo.renderArea.offset = { 0, 0 };
            renderPassInfo.renderArea.extent = m_Swapchain.extent;
            VkClearValue clearColor{ { { 0.0f, 0.0f, 0.0f, 1.0f } } };
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearColor;

            VkViewport viewport = {};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(m_Swapchain.extent.width);
            viewport.height = static_cast<float>(m_Swapchain.extent.height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;

            VkRect2D scissor = {};
            scissor.offset = { 0, 0 };
            scissor.extent = m_Swapchain.extent;

            vkCmdSetViewport(m_CommandBuffers[i], 0, 1, &viewport);
            vkCmdSetScissor(m_CommandBuffers[i], 0, 1, &scissor);

            vkCmdBeginRenderPass(m_CommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);

            vkCmdDraw(m_CommandBuffers[i], 3, 1, 0, 0);

            vkCmdEndRenderPass(m_CommandBuffers[i]);

            ASSERT_CORE_LOG(vkEndCommandBuffer(m_CommandBuffers[i])== VK_SUCCESS,"Failed to record command buffer");
        }
    }
}
