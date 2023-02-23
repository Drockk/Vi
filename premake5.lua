include "./vendor/premake/premake_customization/solution_items.lua"
include "dependencies.lua"

workspace "ViEngine"
    architecture "x86_64"
    startproject "Viking"

    configurations
    {
        "Debug",
        "Release"
    }

    solution_items
    {
        ".editorconfig"
    }

    flags
    {
        "MultiProcessorCompile"
    }

    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

    group "Dependencies"
        -- include "vendor/premake"
        include "Vi/vendor/Box2D"
        include "Vi/vendor/GLFW"
        include "Vi/vendor/Glad"
        include "Vi/vendor/imgui"
        include "Vi/vendor/yaml-cpp"
    group ""

    group "Core"
        -- include "Vi"
        -- include "Vi-ScriptCore"
    group ""

    group "Tools"
        -- include "ViEd"
    group ""

    group "Misc"
        -- include "Sandbox"
    group ""
