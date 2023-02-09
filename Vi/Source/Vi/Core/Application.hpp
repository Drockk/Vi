#pragma once
namespace Vi {
    class Application {
    public:
        Application() = default;
        ~Application() = default;

        void init();
        void run();
        void shutdown();
    };
}
