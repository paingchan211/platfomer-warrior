#include "Game.h"
#include "Session.h"
#include <iostream>

Game &Game::getInstance()
{
    static Game instance;
    return instance;
}

Game::Game()
    : resourceManager(), session(nullptr), running(false) {}

Game::~Game()
{
    session.reset();
}

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

void Game::newGame()
{
    session = std::make_unique<Session>(resourceManager);
    session->run();
}

void Game::exit()
{
    running = false;

    if (session)
    {
        session.reset();
    }
}

bool Game::isRunning() const
{
    return running;
}
