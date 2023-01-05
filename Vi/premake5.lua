project "Vi"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin/int/" .. outputdir .. "/%{prj.name}")

    pchheader "vipch.hpp"
    pchsource "Source/vipch.cpp"

    files
    {
        "Source/**.hpp",
        "Source/**.cpp",
    }

    includedirs
    {
        "Source",
        "%{IncludeDir.glfw}",
        "%{IncludeDir.spdlog}"
    }

    links
    {
        "GLFW",
        "%{Library.Vulkan}"
    }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "GLFW_INCLUDE_NONE"
        }

    filter "configurations:Debug"
        defines "VI_DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "VI_RELEASE"
        runtime "Release"
        optimize "on"