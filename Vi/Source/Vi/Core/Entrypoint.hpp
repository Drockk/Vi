#pragma once
#include "Application.hpp"

#include <memory>

int main() {
    const auto application = std::make_unique<Vi::Application>();

    application->init();
    application->run();
    application->shutdown();

    return 0;
}
