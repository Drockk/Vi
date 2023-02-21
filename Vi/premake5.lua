project "Vi"
    kind "StaticLib"
    language "C++"
    cppdialect "C++latest"
    staticruntime "off"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin/int/" .. outputdir .. "/%{prj.name}")

    pchheader "vipch.hpp"
    pchsource "Source/vipch.cpp"

    files
    {
        "Source/**.hpp",
        "Source/**.cpp"
    }

    includedirs
    {
        "Source",
        "%{IncludeDir.eventpp}",
        "%{IncludeDir.glfw}",
        "%{IncludeDir.spdlog}"
    }

    defines
    {
        "GLFW_INCLUDE_NONE"
    }

    links
    {
        "GLFW"
    }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "VI_PLATFORM_WINDOWS"
        }
    filter "configurations:Debug"
        defines "VI_DEBUG"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines "VI_RELEASE"
        runtime "Release"
        optimize "On"
