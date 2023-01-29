#include "SandboxApplication.hpp"
#include "SandboxLayer.hpp"

SandboxApplication::SandboxApplication(): Application("Sandbox") {
    pushLayer(new SandboxLayer());
}

std::unique_ptr<Vi::Application> createApplication() {
    return std::make_unique<SandboxApplication>();
}
