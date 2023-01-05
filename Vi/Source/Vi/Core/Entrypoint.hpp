#pragma once
#include "Vi/Core/Application.hpp"

extern std::unique_ptr<Vi::Application> createApplication();

int main() {
    const auto app = createApplication();

    app->init();
    app->run();
    app->shutdown();

    return 0;
}
