#include "ViEngine.hpp"

int main(int argc, char* argv[])
{
    ViEngine engine;

    engine.init();

    engine.run();

    engine.cleanup();

    return 0;
}