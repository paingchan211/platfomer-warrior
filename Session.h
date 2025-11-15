#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <random>

#include "Animation.h"
#include "Boss.h"
#include "Constants.h"
#include "FloatingText.h"
#include "Enemy.h"
#include "Platform.h"
#include "Player.h"
#include "ResourceManager.h"
#include "SinglyLinkedList.h"
#include "Stack.h"
#include "Queue.h"
#include "GameState.h"
#include "UISystem.h"
#include "InputManager.h"
#include "CombatSystem.h"
#include "GameWorld.h"
#include "CameraController.h"
#include "PhysicsManager.h"
#include "KeyBindingManager.h"
#include "GameMaster.h"
#include "SaveGameManager.h"

class Session
{
private:
    // ---------- Game Window ----------
    sf::RenderWindow window; // Main render window

    // ---------- Core Systems ----------
    GameWorld gameWorld;                 // Manages entities and world
    CameraController cameraController;   // Controls camera movement
    PhysicsManager physicsManager;       // Manages physics and collisions
    UISystem uiSystem;                   // Combines HUD and menu rendering
    InputManager inputManager;           // Manages player input
    CombatSystem combatSystem;           // Manages combat logic
    KeyBindingManager keyBindingManager; // Handles key mappings
    GameMaster gameMaster;               // Oversees game flow
    SaveGameManager saveGameManager;     // Handles saving/loading game state
    // ---------- Combat and Damage Tracking ----------
    std::unique_ptr<bool[]> enemyAttackDamageApplied; // Tracks enemy attack application
    bool bossAttackDamageApplied = false;             // Tracks if boss attack applied
    bool bossDeathSlowMotionActive = false;           // Enables slow motion on boss death
    float bossDeathSlowMotionTimer = 0.f;             // Timer for slow motion effect

    // ---------- Player & Status ----------
    float lowHpWarningTimer = 0.f; // Timer for low HP warnings

    // ---------- Floating Texts ----------
    Queue<std::unique_ptr<FloatingText>> floatingTexts; // Queue of floating text entities

    // ---------- Combat Log ----------
    SinglyLinkedList<std::string> combatLog;       // Combat message log
    static const int MAX_COMBAT_LOG_ENTRIES = 100; // Max number of log entries
    int combatLogCurrentNode = 0;                  // Current node index
    int combatLogTraversalCount = 0;               // Traversal counter
    int combatLogDeleteCount = 0;                  // Deletion counter

    // ---------- Debugging ----------
    bool debugMode = false;                                   // Enables debug mode
    std::string lastPressedKeyName = "";                      // Last key pressed (string)
    sf::Keyboard::Key lastPressedKey = sf::Keyboard::Unknown; // Last key pressed (enum)

    // ---------- Game State Management ----------
    Stack<GameStateData> stateStack;  // Stack of game states
    bool gameOver = false;            // Game-over flag
    bool gameWon = false;             // Game-won flag
    bool gameOverSoundPlayed = false; // Prevents replaying game-over sound

    // ---------- Timing and Randomness ----------
    sf::Clock deltaClock;                     // Tracks delta time between frames
    std::mt19937 rng{std::random_device{}()}; // Random generator

    // ---------- Exit Control ----------
    bool requestExit = false;                 // Exit request flag
    bool combatLogStdoutEnabled = ENABLE_COMBAT_LOG_STDOUT; // Mirrors combat log operations to std::cout when true

public:
    // ---------- Constructor & Destructor ----------
    Session();  // Constructor
    ~Session(); // Destructor

    // ---------- Main Game Loop ----------
    void run();              // Runs the main game loop
    bool shouldExit() const; // Checks if game should exit
    void setCombatLogStdoutEnabled(bool enabled);
    bool isCombatLogStdoutEnabled() const { return combatLogStdoutEnabled; }

private:
    // ---------- Initialization ----------
    bool initialize(); // Initializes systems and assets

    // ---------- Game Loop Steps ----------
    void processEvents();  // Handles input/events
    void update(float dt); // Updates game logic
    void render();         // Draws everything on screen

    // ---------- Update Functions ----------
    void updatePlayer(float dt);        // Updates player
    void updateEnemies(float dt);       // Updates enemies
    void updateBoss(float dt);          // Updates boss
    void updateFloatingTexts(float dt); // Updates floating texts

    // ---------- Game Control ----------
    void resetGame();     // Resets the game state (now calls gameWorld.reset(entityFactory))
    void checkGameOver(); // Checks for win/lose state

    // ---------- Rendering ----------
    void renderWorld();           // Renders game world
    void renderGameOver();        // Displays game-over UI
    void renderDebugCollisions(); // Shows collision outlines
    void renderKeyDisplay();      // Displays input info in debug
    void renderStateStack();      // Shows state stack in debug

    // ---------- Combat Log ----------
    void addCombatLog(const std::string &message); // Adds combat log entry
    void clearCombatLog();                         // Clears all combat logs

    // ---------- State Management ----------
    void pushState(GameStateType type);    // Pushes new state
    void popState();                       // Pops top state
    void changeState(GameStateType type);  // Replaces top state
    GameStateType getCurrentState() const; // Gets current state
    bool isGamePaused() const;             // Checks if game paused

    // ---------- Floating Text Control ----------
    void spawnFloatingText(int damage, sf::Vector2f position, const sf::Color &color = sf::Color::Red, bool isHealing = false); // Creates floating text
    void clearFloatingTexts(const char *context = nullptr);                                                                     // Clears floating texts
};
