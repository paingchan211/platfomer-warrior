#pragma once

#include <memory>

class Session;

// Main Game class responsible for managing sessions and state
class Game
{
public:
    Game();  // Constructor
    ~Game(); // Destructor

    void run();     // Runs the main game
    void newGame(); // Starts a new game session
    void exit();    // Exits the game and cleans up

    bool isRunning() const; // Returns whether the game is active

private:
    std::unique_ptr<Session> session; // Pointer to the current game session
    bool running;                     // Tracks if the game is currently running
};