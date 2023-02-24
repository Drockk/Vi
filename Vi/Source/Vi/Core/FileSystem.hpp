#pragma once
#include "Vi/Core/Buffer.hpp"

namespace Vi {
    class FileSystem {
    public:
        // TODO: move to FileSystem class
        static Buffer readFileBinary(const std::filesystem::path& filepath);
    };
}
