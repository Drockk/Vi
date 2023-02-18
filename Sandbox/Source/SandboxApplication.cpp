#include <Vi.hpp>
#include <Vi/Core/Entrypoint.hpp>

class SandboxApplication: public Vi::Application {
public:
    SandboxApplication() = default;
    ~SandboxApplication() override = default;
};

std::unique_ptr<Vi::Application> createApplication() {
    return std::make_unique<SandboxApplication>();
}
