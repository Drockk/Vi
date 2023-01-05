include "dependencies.lua"

workspace "Vi"
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

    include "Vi"
    include "Sandbox"
