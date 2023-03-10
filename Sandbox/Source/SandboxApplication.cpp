#include <Vi.hpp>
#include <Vi/Core/Entrypoint.hpp>

#include "SandboxLayer.hpp"

class SandboxApplication: public Vi::Application {
public:
    SandboxApplication(): Application("Sandbox") {}
    ~SandboxApplication() override = default;
};

std::unique_ptr<Vi::Application> createApplication() {
    return std::make_unique<SandboxApplication>();
}
