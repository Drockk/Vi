#pragma once
#include "Application.hpp"

#include <memory>

extern std::unique_ptr<Vi::Application> createApplication();

int main() {
    auto application = createApplication();

    application->init();
    application->run();
    application->shutdown();

    return 0;
}
