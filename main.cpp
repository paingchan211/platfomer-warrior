#include "Game.h"
#include <iostream>

// Entry point — creates and runs the game, with basic error reporting
int main()
{
    try
    {
        Game game;  // Construct game object
        game.run(); // Enter main game loop
    }
    catch (const std::exception &e)
    {
        // Print any unhandled exception and exit with error code
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0; // Normal exit
}