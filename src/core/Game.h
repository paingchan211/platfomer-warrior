#pragma once

#include <memory>

#include "ResourceManager.h"

class Session;

class Game
{
public:
    // Returns the single global instance of the Game (Singleton pattern)
    static Game &getInstance();

    // Starts the main game loop
    void run();

    // Creates a new game session and resets all related state
    void newGame();

    // Signals the game to stop running and begin shutdown
    void exit();

    // Returns whether the main game loop is currently active
    bool isRunning() const;

private:
    // Private constructor ensures only getInstance() can create the object
    Game();

    // Destructor cleans up resources such as the Session
    ~Game();

    // Deleted copy constructor (Singleton should not be copied)
    Game(const Game &) = delete;

    // Deleted copy assignment operator
    Game &operator=(const Game &) = delete;

    // Deleted move constructor (prevents transferring Singleton state)
    Game(Game &&) = delete;

    // Deleted move assignment operator
    Game &operator=(Game &&) = delete;

    // Manages loading and caching of textures, fonts, and other resources
    ResourceManager resourceManager;

    // Pointer to the active session containing gameplay state
    std::unique_ptr<Session> session;

    // Tracks whether the game loop should continue running
    bool running;
};