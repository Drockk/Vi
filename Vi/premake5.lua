project "Vi"
    kind "ConsoleApp"
    language "C++"
    cppdialect "c++latest"
    staticruntime "off"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin/int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "source/**.hpp",
        "source/**.cpp",
        "%{IncludeDir.vkBootstrap}/VkBootstrap.h",
        "%{IncludeDir.vkBootstrap}/VkBootstrapDispatch.h",
        "%{IncludeDir.vkBootstrap}/VkBootstrap.cpp"
    }

    includedirs
    {
        "source",
        "%{IncludeDir.glfw}",
        "%{IncludeDir.VulkanSDK}",
        "%{IncludeDir.vkBootstrap}"
    }

    links
    {
        "GLFW",
        "%{Library.Vulkan}"
    }

    defines
    {
        "GLFW_INCLUDE_NONE"
    }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        defines "VI_DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "VI_RELEASE"
        runtime "Release"
        optimize "on"
