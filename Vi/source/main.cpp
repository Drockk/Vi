#include "vk_engine.hpp"

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    VulkanEngine engine;
    engine.init();
    engine.run();
    engine.cleanup();

    return 0;
}
