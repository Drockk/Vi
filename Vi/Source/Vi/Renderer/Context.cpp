#include "vipch.hpp"
#include "Vi/Renderer/Context.hpp"

namespace Vi {
    void Context::init(const std::shared_ptr<Window>& window) {
        initInstance(window->getName());
        initSurface(window);
        initDevice();
        initSwapchain();

        getGraphicsQueue();
        getPresentQueue();

        createRenderpass();
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
}
