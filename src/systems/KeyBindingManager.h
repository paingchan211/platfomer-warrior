#pragma once

#include <SFML/Window/Keyboard.hpp>
#include <string>

// All configurable game actions
enum class GameAction
{
    MOVE_LEFT,
    MOVE_RIGHT,
    JUMP,
    SHOOT_PROJECTILE,
    SWITCH_PROJECTILE,
    MELEE_ATTACK,
    SPECIAL_ATTACK,
    USE_POTION,
    OPEN_SKILL_TREE,
    ACTION_COUNT // Sentinel for array sizing
};

// Manages keyboard bindings for gameplay actions
class KeyBindingManager
{
public:
    KeyBindingManager();  // Constructor
    ~KeyBindingManager(); // Destructor

    sf::Keyboard::Key getKeyForAction(GameAction action) const;              // Returns bound key
    bool isKeyBoundToAction(GameAction action, sf::Keyboard::Key key) const; // Checks exact binding
    bool rebindAction(GameAction action, sf::Keyboard::Key newKey);          // Rebinds an action

    bool isKeyAlreadyBound(sf::Keyboard::Key key, GameAction &conflictingAction) const; // Detects conflicts

    void resetToDefaults();                               // Restores default bindings
    bool saveBindings(const std::string &filename) const; // Saves to config file
    bool loadBindings(const std::string &filename);       // Loads from config file

    static std::string getActionName(GameAction action);  // Pretty action name
    static std::string getKeyName(sf::Keyboard::Key key); // Pretty key name

private:
    sf::Keyboard::Key keyBindings[static_cast<int>(GameAction::ACTION_COUNT)]; // Key array by action index
    void initializeDefaults();                                                 // Fills defaults
};