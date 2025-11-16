#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include "GameState.h"
#include "Constants.h"
#include "SinglyLinkedList.h"
#include "Stack.h"
#include "ResourceManager.h"
#include "KeyBindingManager.h"

class Player;
class UISystem;

// Manages keyboard input and state transitions for menus and gameplay
class InputManager
{
public:
    InputManager();  // Constructor
    ~InputManager(); // Destructor

    // Main event processing entry point
    void processEvent(const sf::Event &event,
                      Stack<GameStateData> &stateStack,
                      bool gameOver,
                      Player *player,
                      ResourceManager &resourceManager,
                      SinglyLinkedList<std::string> &combatLog,
                      int &combatLogCurrentNode,
                      int &combatLogTraversalCount,
                      int &combatLogDeleteCount,
                      bool &requestExit,
                      UISystem *uiSystem,
                      KeyBindingManager *keyManager = nullptr,
                      class SaveGameManager *saveManager = nullptr);

    // Returns whether a specific SFML key is currently pressed
    bool isKeyDown(sf::Keyboard::Key key) const;

    // Resets all stored key states
    void resetKeyStates();

    // Returns if a game action is currently pressed (based on key bindings)
    bool isActionPressed(GameAction action, const KeyBindingManager *keyManager = nullptr) const;
    bool isActionDown(GameAction action, const KeyBindingManager *keyManager = nullptr) const;

    // Key state getters
    bool isWalkLeftDown() const;
    bool isWalkRightDown() const;
    bool isProjectileKeyPressed() const;
    bool isSwitchProjectileKeyPressed() const;
    bool isAttack1KeyPressed() const;
    bool isAttack2KeyPressed() const;
    bool isInventoryKeyDown() const;

    // Key state setters
    void setProjectileKeyPressed(bool pressed);
    void setSwitchProjectileKeyPressed(bool pressed);
    void setAttack1KeyPressed(bool pressed);
    void setAttack2KeyPressed(bool pressed);
    void setCombatLogStdoutEnabled(bool enabled);

private:
    // Internal helper functions for each menu/game mode
    void handleMainMenuInput(const sf::Event &event,
                             Stack<GameStateData> &stateStack,
                             bool &requestExit,
                             class SaveGameManager *saveManager = nullptr);

    void handleGameInput(const sf::Event &event,
                         Stack<GameStateData> &stateStack,
                         bool gameOver,
                         ResourceManager &resourceManager,
                         KeyBindingManager *keyManager = nullptr);

    void handlePauseMenuInput(const sf::Event &event,
                              Stack<GameStateData> &stateStack,
                              SinglyLinkedList<std::string> &combatLog,
                              int &combatLogCurrentNode,
                              int &combatLogTraversalCount,
                              int &combatLogDeleteCount);

    void handleSettingsMenuInput(const sf::Event &event,
                                 Stack<GameStateData> &stateStack);

    void handleControlsMenuInput(const sf::Event &event,
                                 Stack<GameStateData> &stateStack,
                                 KeyBindingManager *keyManager,
                                 UISystem *uiSystem);

    void handleAudioSettingsInput(const sf::Event &event,
                                  Stack<GameStateData> &stateStack,
                                  ResourceManager &resourceManager);

    void handleConfirmDialogInput(const sf::Event &event,
                                  Stack<GameStateData> &stateStack,
                                  Player *player,
                                  class SaveGameManager *saveManager = nullptr,
                                  UISystem *uiSystem = nullptr);

    void handleSaveGameMenuInput(const sf::Event &event,
                                 Stack<GameStateData> &stateStack,
                                 Player *player,
                                 class SaveGameManager *saveManager,
                                 UISystem *uiSystem);

    void handleLoadGameMenuInput(const sf::Event &event,
                                 Stack<GameStateData> &stateStack,
                                 Player *player,
                                 class SaveGameManager *saveManager,
                                 UISystem *uiSystem);

    void handleSimpleScreenInput(const sf::Event &event,
                                 Stack<GameStateData> &stateStack,
                                 Player *player,
                                 SinglyLinkedList<std::string> &combatLog,
                                 UISystem *uiSystem,
                                 KeyBindingManager *keyManager = nullptr);

    void handleCombatLogInput(const sf::Event &event,
                              Stack<GameStateData> &stateStack,
                              SinglyLinkedList<std::string> &combatLog,
                              int &combatLogCurrentNode,
                              int &combatLogTraversalCount,
                              int &combatLogDeleteCount);

    void appendCombatLogEntry(SinglyLinkedList<std::string> &combatLog, const std::string &message);

    // Key state flags
    bool leftKeyDown;
    bool rightKeyDown;
    bool enterKeyDown;
    bool walkLeftDown;
    bool walkRightDown;
    bool escKeyDown;
    bool upKeyDown;
    bool downKeyDown;
    bool inventoryKeyDown;
    bool projectileKeyPressed;
    bool switchProjectileKeyPressed;
    bool attack1KeyPressed;
    bool attack2KeyPressed;
    bool combatLogStdoutEnabled;
};
