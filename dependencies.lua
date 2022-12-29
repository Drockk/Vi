-- Viking dependencies

VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}
IncludeDir["glfw"] = "%{wks.location}/Vi/Vendor/glfw/include"
IncludeDir["glm"] = "%{wks.location}/Vi/Vendor/glm"
IncludeDir["vma"] = "%{wks.location}/Vi/Vendor/VulkanMemoryAllocator/include"
IncludeDir["vkbootstrap"] = "%{wks.location}/Vi/Vendor/vk-bootstrap/src"
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"

LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"

Library = {}
Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["VulkanUtils"] = "%{LibraryDir.VulkanSDK}/VkLayer_utils.lib"