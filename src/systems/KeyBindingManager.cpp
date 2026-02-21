#include "KeyBindingManager.h"
#include <fstream>
#include <sstream>

// Constructor — initialize with default bindings
KeyBindingManager::KeyBindingManager()
{
    initializeDefaults();
}

// Destructor — nothing to release
KeyBindingManager::~KeyBindingManager()
{
}

// Initializes default key bindings for all actions
void KeyBindingManager::initializeDefaults()
{
    // Movement
    keyBindings[static_cast<int>(GameAction::MOVE_LEFT)] = sf::Keyboard::Left;
    keyBindings[static_cast<int>(GameAction::MOVE_RIGHT)] = sf::Keyboard::Right;
    keyBindings[static_cast<int>(GameAction::JUMP)] = sf::Keyboard::Space;

    // Combat
    keyBindings[static_cast<int>(GameAction::SHOOT_PROJECTILE)] = sf::Keyboard::F;
    keyBindings[static_cast<int>(GameAction::SWITCH_PROJECTILE)] = sf::Keyboard::R;
    keyBindings[static_cast<int>(GameAction::MELEE_ATTACK)] = sf::Keyboard::A;
    keyBindings[static_cast<int>(GameAction::SPECIAL_ATTACK)] = sf::Keyboard::S;

    // Items / Menus
    keyBindings[static_cast<int>(GameAction::USE_POTION)] = sf::Keyboard::Q;
    keyBindings[static_cast<int>(GameAction::OPEN_SKILL_TREE)] = sf::Keyboard::T;
}

// Returns the bound key for a game action (or Unknown)
sf::Keyboard::Key KeyBindingManager::getKeyForAction(GameAction action) const
{
    int index = static_cast<int>(action);
    if (index >= 0 && index < static_cast<int>(GameAction::ACTION_COUNT))
        return keyBindings[index];
    return sf::Keyboard::Unknown;
}

// Returns true if the given key is bound to the specified action
bool KeyBindingManager::isKeyBoundToAction(GameAction action, sf::Keyboard::Key key) const
{
    return getKeyForAction(action) == key;
}

// Returns true if the key is already bound to any action, and outputs which
bool KeyBindingManager::isKeyAlreadyBound(sf::Keyboard::Key key, GameAction &conflictingAction) const
{
    for (int i = 0; i < static_cast<int>(GameAction::ACTION_COUNT); ++i)
    {
        if (keyBindings[i] == key)
        {
            conflictingAction = static_cast<GameAction>(i);
            return true;
        }
    }
    return false;
}

// Rebinds an action to a new key (fails if the key conflicts with another action)
bool KeyBindingManager::rebindAction(GameAction action, sf::Keyboard::Key newKey)
{
    // Prevent binding a key already used by a different action
    GameAction conflict;
    if (isKeyAlreadyBound(newKey, conflict) && conflict != action)
        return false;

    int index = static_cast<int>(action);
    if (index >= 0 && index < static_cast<int>(GameAction::ACTION_COUNT))
    {
        keyBindings[index] = newKey;
        return true;
    }
    return false;
}

// Restores the default bindings
void KeyBindingManager::resetToDefaults()
{
    initializeDefaults();
}

// Saves bindings to a simple config file (ACTION_NAME=KEY_CODE)
bool KeyBindingManager::saveBindings(const std::string &filename) const
{
    std::ofstream file(filename);
    if (!file.is_open())
        return false;

    file << "# Key Bindings Configuration\n";
    file << "# Format: ACTION_NAME=KEY_CODE\n\n";

    for (int i = 0; i < static_cast<int>(GameAction::ACTION_COUNT); ++i)
    {
        GameAction action = static_cast<GameAction>(i);
        sf::Keyboard::Key key = keyBindings[i];
        file << getActionName(action) << "=" << static_cast<int>(key) << "\n";
    }

    file.close();
    return true;
}

// Loads bindings from a config file; keeps defaults for any missing entries
bool KeyBindingManager::loadBindings(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
        return false;

    // Temp array initialized to Unknown to detect missing entries
    sf::Keyboard::Key tempBindings[static_cast<int>(GameAction::ACTION_COUNT)];
    bool loadedAny = false;
    for (int i = 0; i < static_cast<int>(GameAction::ACTION_COUNT); ++i)
        tempBindings[i] = sf::Keyboard::Unknown;

    std::string line;
    while (std::getline(file, line))
    {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#')
            continue;

        // Parse ACTION_NAME=KEY_CODE
        size_t equalPos = line.find('=');
        if (equalPos == std::string::npos)
            continue;

        std::string actionName = line.substr(0, equalPos);
        int keyCode;
        try
        {
            keyCode = std::stoi(line.substr(equalPos + 1));
        }
        catch (...)
        {
            continue;
        }

        // Map action name to enum
        GameAction action;
        bool found = false;

        if (actionName == "MOVE_LEFT")
        {
            action = GameAction::MOVE_LEFT;
            found = true;
        }
        else if (actionName == "MOVE_RIGHT")
        {
            action = GameAction::MOVE_RIGHT;
            found = true;
        }
        else if (actionName == "JUMP")
        {
            action = GameAction::JUMP;
            found = true;
        }
        else if (actionName == "SHOOT_PROJECTILE")
        {
            action = GameAction::SHOOT_PROJECTILE;
            found = true;
        }
        else if (actionName == "SWITCH_PROJECTILE")
        {
            action = GameAction::SWITCH_PROJECTILE;
            found = true;
        }
        else if (actionName == "MELEE_ATTACK")
        {
            action = GameAction::MELEE_ATTACK;
            found = true;
        }
        else if (actionName == "SPECIAL_ATTACK")
        {
            action = GameAction::SPECIAL_ATTACK;
            found = true;
        }
        else if (actionName == "USE_POTION")
        {
            action = GameAction::USE_POTION;
            found = true;
        }
        else if (actionName == "OPEN_SKILL_TREE")
        {
            action = GameAction::OPEN_SKILL_TREE;
            found = true;
        }

        // Store loaded key when recognized
        if (found)
        {
            tempBindings[static_cast<int>(action)] = static_cast<sf::Keyboard::Key>(keyCode);
            loadedAny = true;
        }
    }
    file.close();

    // If nothing valid loaded, keep defaults
    if (!loadedAny)
    {
        initializeDefaults();
        return false;
    }

    // Apply any loaded keys; leave others at defaults
    for (int i = 0; i < static_cast<int>(GameAction::ACTION_COUNT); ++i)
    {
        if (tempBindings[i] != sf::Keyboard::Unknown)
            keyBindings[i] = tempBindings[i];
    }
    return true;
}

// Returns the canonical action name (used in UI/config)
std::string KeyBindingManager::getActionName(GameAction action)
{
    switch (action)
    {
    case GameAction::MOVE_LEFT:
        return "MOVE_LEFT";
    case GameAction::MOVE_RIGHT:
        return "MOVE_RIGHT";
    case GameAction::JUMP:
        return "JUMP";
    case GameAction::SHOOT_PROJECTILE:
        return "SHOOT_PROJECTILE";
    case GameAction::SWITCH_PROJECTILE:
        return "SWITCH_PROJECTILE";
    case GameAction::MELEE_ATTACK:
        return "MELEE_ATTACK";
    case GameAction::SPECIAL_ATTACK:
        return "SPECIAL_ATTACK";
    case GameAction::USE_POTION:
        return "USE_POTION";
    case GameAction::OPEN_SKILL_TREE:
        return "OPEN_SKILL_TREE";
    default:
        return "UNKNOWN";
    }
}

// Pretty-prints a keyboard key name for UI
std::string KeyBindingManager::getKeyName(sf::Keyboard::Key key)
{
    switch (key)
    {
    // Letters
    case sf::Keyboard::A:
        return "A";
    case sf::Keyboard::B:
        return "B";
    case sf::Keyboard::C:
        return "C";
    case sf::Keyboard::D:
        return "D";
    case sf::Keyboard::E:
        return "E";
    case sf::Keyboard::F:
        return "F";
    case sf::Keyboard::H:
        return "H";
    case sf::Keyboard::I:
        return "I";
    case sf::Keyboard::J:
        return "J";
    case sf::Keyboard::K:
        return "K";
    case sf::Keyboard::L:
        return "L";
    case sf::Keyboard::M:
        return "M";
    case sf::Keyboard::N:
        return "N";
    case sf::Keyboard::O:
        return "O";
    case sf::Keyboard::P:
        return "P";
    case sf::Keyboard::Q:
        return "Q";
    case sf::Keyboard::R:
        return "R";
    case sf::Keyboard::S:
        return "S";
    case sf::Keyboard::T:
        return "T";
    case sf::Keyboard::U:
        return "U";
    case sf::Keyboard::V:
        return "V";
    case sf::Keyboard::W:
        return "W";
    case sf::Keyboard::X:
        return "X";
    case sf::Keyboard::Y:
        return "Y";
    case sf::Keyboard::Z:
        return "Z";

    // Numbers
    case sf::Keyboard::Num0:
        return "0";
    case sf::Keyboard::Num1:
        return "1";
    case sf::Keyboard::Num2:
        return "2";
    case sf::Keyboard::Num3:
        return "3";
    case sf::Keyboard::Num4:
        return "4";
    case sf::Keyboard::Num5:
        return "5";
    case sf::Keyboard::Num6:
        return "6";
    case sf::Keyboard::Num7:
        return "7";
    case sf::Keyboard::Num8:
        return "8";
    case sf::Keyboard::Num9:
        return "9";

    // Controls / navigation
    case sf::Keyboard::Escape:
        return "ESC";
    case sf::Keyboard::Space:
        return "SPACE";
    case sf::Keyboard::Enter:
        return "ENTER";
    case sf::Keyboard::Tab:
        return "TAB";
    case sf::Keyboard::Backspace:
        return "BACKSPACE";
    case sf::Keyboard::Delete:
        return "DELETE";
    case sf::Keyboard::Left:
        return "LEFT";
    case sf::Keyboard::Right:
        return "RIGHT";
    case sf::Keyboard::Up:
        return "UP";
    case sf::Keyboard::Down:
        return "DOWN";
    case sf::Keyboard::LShift:
        return "L-SHIFT";
    case sf::Keyboard::RShift:
        return "R-SHIFT";
    case sf::Keyboard::LControl:
        return "L-CTRL";
    case sf::Keyboard::RControl:
        return "R-CTRL";

    // Fallback for less common keys
    default:
        return "KEY_" + std::to_string(static_cast<int>(key));
    }
}
