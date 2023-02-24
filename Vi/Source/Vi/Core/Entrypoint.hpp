#pragma once
#include "Vi/Core/Base.hpp"
#include "Vi/Core/Application.hpp"

#ifdef VI_PLATFORM_WINDOWS

extern Vi::Application* Vi::createApplication(ApplicationCommandLineArgs args);

int main(int argc, char** argv)
{
    Vi::Log::init();

    VI_PROFILE_BEGIN_SESSION("Startup", "ViProfile-Startup.json");
    auto app = Vi::createApplication({ argc, argv });
    VI_PROFILE_END_SESSION();

    VI_PROFILE_BEGIN_SESSION("Runtime", "ViProfile-Runtime.json");
    app->run();
    VI_PROFILE_END_SESSION();

    VI_PROFILE_BEGIN_SESSION("Shutdown", "ViProfile-Shutdown.json");
    delete app;
    VI_PROFILE_END_SESSION();
}

#endif
