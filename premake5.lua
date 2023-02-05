include "dependencies.lua"

workspace "Vi Engine"
    architecture "x86_64"
    startproject "Vi"

    configurations {
        "Debug",
        "Release"
    }

    flags
    {
        "MultiProcessorCompile"
    }

    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

    group "Dependencies"
        include "Vi/vendor/glfw"
    group ""

    include "Vi"
