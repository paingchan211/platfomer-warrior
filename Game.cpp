#include "Game.h"
#include "Session.h"
#include <iostream>

// Default constructor initializing session and running state
Game::Game()
    : session(nullptr), running(false) {}

// Destructor that cleans up the session
Game::~Game()
{
    session.reset();
}

// Runs the main game logic safely within a try-catch block
void Game::run()
{
    try
    {
        running = true;
        newGame(); // Start a new game session
        running = false;
    }
    catch (const std::exception &e)
    {
        // Catch and log any runtime errors
        std::cerr << "Game Error: " << e.what() << std::endl;
        running = false;
    }
}

// Starts a new game by creating a new session
void Game::newGame()
{
    session = std::make_unique<Session>();
    session->run();
}

// Stops the game and cleans up the session
void Game::exit()
{
    running = false;

    if (session)
    {
        session.reset();
    }
}

// Returns true if the game is currently running
bool Game::isRunning() const
{
    return running;
}