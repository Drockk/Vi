#pragma once
//STD
#include <memory>
#include <string>

#include <Vi.hpp>
#include <Vi/Core/Entrypoint.hpp>

class SandboxApplication: public Vi::Application {
public:
    SandboxApplication();
    ~SandboxApplication() = default;
};
