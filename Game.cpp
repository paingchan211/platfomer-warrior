#include "Game.h"
#include "Session.h"
#include <iostream>

Game &Game::getInstance()
{
    // Static local instance ensures only one Game object exists (Singleton)
    static Game instance;
    return instance;
}

Game::Game()
    : resourceManager(), session(nullptr), running(false) {}
// Initialize resource manager, set no active session, and mark game as not running

Game::~Game()
{
    // Clean up the active session if it exists
    session.reset();
}

void Game::run()
{
    try
    {
        // Mark game as running before starting a session
        running = true;

        // Create and run a new session
        newGame();

        // When session ends normally, mark game as not running
        running = false;
    }
    catch (const std::exception &e)
    {
        // Log any unexpected runtime errors during game execution
        std::cerr << "Game Error: " << e.what() << std::endl;

        // Ensure game stops running after an exception
        running = false;
    }
}

void Game::newGame()
{
    // Create a new gameplay session, passing in the shared resource manager
    session = std::make_unique<Session>(resourceManager);

    // Start running the session loop
    session->run();
}

void Game::exit()
{
    // Signal the game loop to stop
    running = false;

    // Destroy the session if one is active
    if (session)
    {
        session.reset();
    }
}

bool Game::isRunning() const
{
    // Returns whether the main game loop is still active
    return running;
}