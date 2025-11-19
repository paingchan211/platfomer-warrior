#include "Game.h"
#include "Constants.h"
#include <iostream>

// Entry point - creates and runs the game, with basic error reporting
int main()
{
    // Runtime proof that Game is a Singleton:
    // two calls to getInstance() must return the same address.
    if (ENABLE_SINGLETON_STDOUT)
    {
        auto &g1 = Game::getInstance();
        auto &g2 = Game::getInstance();
        std::cout << "[Singleton] Address check: g1=" << &g1
                  << ", g2=" << &g2 << std::endl;
    }

    try
    {
        Game::getInstance().run(); // create and run the game
    }
    catch (const std::exception &e)
    {
        // Print any unhandled exception and exit with error code
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0; // Normal exit
}
