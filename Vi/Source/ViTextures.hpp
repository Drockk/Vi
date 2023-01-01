// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <ViTypes.hpp>
#include <ViEngine.hpp>

namespace viutil {
    bool loadImageFromFile(ViEngine& engine, const char* file, AllocatedImage& outImage);
}