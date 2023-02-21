project "Sandbox"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++latest"
    staticruntime "off"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin/int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "Source/**.hpp",
        "Source/**.cpp"
    }

    includedirs
    {
        "%{wks.location}/Vi/Source",
        "%{IncludeDir.eventpp}",
        "%{IncludeDir.glfw}",
        "%{IncludeDir.spdlog}"
    }

    links
    {
        "Vi"
    }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "VI_PLATFORM_WINDOWS",
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
