#pragma once

#include <memory>

#include "ResourceManager.h"

class Session;

class Game
{
public:
    static Game &getInstance();

    void run();
    void newGame();
    void exit();

    bool isRunning() const;

private:
    Game();
    ~Game();

    Game(const Game &) = delete;
    Game &operator=(const Game &) = delete;
    Game(Game &&) = delete;
    Game &operator=(Game &&) = delete;

    ResourceManager resourceManager;
    std::unique_ptr<Session> session;
    bool running;
};
