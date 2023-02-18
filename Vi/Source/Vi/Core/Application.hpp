#pragma once
#include <memory>

namespace Vi {
    class Application {
    public:
        Application() = default;
        virtual ~Application() = default;

        void init();
        void run();
        void shutdown();
    };
}

std::unique_ptr<Vi::Application> createApplication();
