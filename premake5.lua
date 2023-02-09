workspace "Vi Engine"
    architecture "x86_64"
    startproject "Viking"

    configurations
    {
        "Debug",
        "Release"
    }

    flags
    {
        "MultiProcessorCompile"
    }

    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

    group "Dependencies"
        include "Vi/Vendor/glfw"
    group ""

    include "Sandbox"
    include "Vi"
