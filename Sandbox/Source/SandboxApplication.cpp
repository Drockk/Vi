#include "SandboxApplication.hpp"

SandboxApplication::SandboxApplication(): Application("Sandbox") {

}

std::unique_ptr<Vi::Application> createApplication() {
    return std::make_unique<SandboxApplication>();
}
