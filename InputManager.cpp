#include "InputManager.h"
#include "Player.h"
#include "ResourceManager.h"
#include "UISystem.h"
#include "SaveGameManager.h"
#include <algorithm>

// Constructor — initialize all key state flags
InputManager::InputManager()
    : leftKeyDown(false),
      rightKeyDown(false),
      enterKeyDown(false),
      walkLeftDown(false),
      walkRightDown(false),
      escKeyDown(false),
      upKeyDown(false),
      downKeyDown(false),
      inventoryKeyDown(false),
      projectileKeyPressed(false),
      switchProjectileKeyPressed(false),
      attack1KeyPressed(false),
      attack2KeyPressed(false)
{
}

// Destructor — nothing to release
InputManager::~InputManager()
{
}

// Resets all tracked key states (prevents sticky keys across screens)
void InputManager::resetKeyStates()
{
    leftKeyDown = false;
    rightKeyDown = false;
    enterKeyDown = false;
    walkLeftDown = false;
    walkRightDown = false;
    escKeyDown = false;
    upKeyDown = false;
    downKeyDown = false;
    inventoryKeyDown = false;
    projectileKeyPressed = false;
    switchProjectileKeyPressed = false;
    attack1KeyPressed = false;
    attack2KeyPressed = false;
}

// Returns current state of a specific SFML key (based on internal flags)
bool InputManager::isKeyDown(sf::Keyboard::Key key) const
{
    switch (key)
    {
    case sf::Keyboard::Left:
        return leftKeyDown;
    case sf::Keyboard::Right:
        return rightKeyDown;
    case sf::Keyboard::Enter:
        return enterKeyDown;
    case sf::Keyboard::Escape:
        return escKeyDown;
    case sf::Keyboard::Up:
        return upKeyDown;
    case sf::Keyboard::Down:
        return downKeyDown;
    case sf::Keyboard::I:
        return inventoryKeyDown;
    default:
        return false;
    }
}

// Returns true if a bound game action key is currently pressed (polls SFML)
bool InputManager::isActionPressed(GameAction action, const KeyBindingManager *keyManager) const
{
    if (!keyManager)
        return false;

    sf::Keyboard::Key key = keyManager->getKeyForAction(action);
    return sf::Keyboard::isKeyPressed(key);
}

// Returns true if a bound game action is flagged as down in our state
bool InputManager::isActionDown(GameAction action, const KeyBindingManager *keyManager) const
{
    if (!keyManager)
        return false;

    sf::Keyboard::Key key = keyManager->getKeyForAction(action);
    return isKeyDown(key);
}

// Main event handler — routes input to the correct screen/state logic
void InputManager::processEvent(const sf::Event &event,
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
                                KeyBindingManager *keyManager,
                                class SaveGameManager *saveManager)
{
    // Key pressed branch
    if (event.type == sf::Event::KeyPressed)
    {
        // If game over, allow quick restart with 'R'
        if (gameOver && event.key.code == sf::Keyboard::R)
        {
            // Clear stack back to Main Menu
            while (!stateStack.isEmpty())
            {
                stateStack.pop();
            }
            stateStack.push(GameStateData(GameStateType::MainMenu));

            // Avoid sticky key states on restart
            leftKeyDown = rightKeyDown = enterKeyDown = false;
        }
        else
        {
            // Determine current state (default to Main Menu)
            GameStateType currentState = GameStateType::MainMenu;
            if (!stateStack.isEmpty())
            {
                currentState = stateStack.top().type;
            }

            // Route to per-state handlers
            if (currentState == GameStateType::MainMenu)
            {
                handleMainMenuInput(event, stateStack, requestExit, saveManager);
            }
            else if (currentState == GameStateType::Paused)
            {
                handlePauseMenuInput(event, stateStack, combatLog, combatLogCurrentNode, combatLogTraversalCount, combatLogDeleteCount);
            }
            else if (currentState == GameStateType::PlayerStatsScreen ||
                     currentState == GameStateType::InventoryOnlyScreen ||
                     currentState == GameStateType::SkillTreeScreen ||
                     currentState == GameStateType::HelpScreen)
            {
                handleSimpleScreenInput(event, stateStack, player, combatLog, uiSystem, keyManager);
            }
            else if (currentState == GameStateType::CombatLogScreen)
            {
                handleCombatLogInput(event, stateStack, combatLog, combatLogCurrentNode, combatLogTraversalCount, combatLogDeleteCount);
            }
            else if (currentState == GameStateType::SettingsMenu)
            {
                handleSettingsMenuInput(event, stateStack);
            }
            else if (currentState == GameStateType::ControlsMenu)
            {
                handleControlsMenuInput(event, stateStack, keyManager, uiSystem);
            }
            else if (currentState == GameStateType::AudioSettings)
            {
                handleAudioSettingsInput(event, stateStack, resourceManager);
            }
            else if (currentState == GameStateType::ConfirmQuitToMenu ||
                     currentState == GameStateType::ConfirmRestart ||
                     currentState == GameStateType::ConfirmOverwriteSave)
            {
                handleConfirmDialogInput(event, stateStack, player, saveManager, uiSystem);
            }
            else if (currentState == GameStateType::SaveGameMenu)
            {
                handleSaveGameMenuInput(event, stateStack, player, saveManager, uiSystem);
            }
            else if (currentState == GameStateType::LoadGameMenu)
            {
                handleLoadGameMenuInput(event, stateStack, player, saveManager, uiSystem);
            }
            else if (currentState == GameStateType::Playing)
            {
                handleGameInput(event, stateStack, gameOver, keyManager);
            }
        }
    }
    // Key released branch — clear flags
    else if (event.type == sf::Event::KeyReleased)
    {
        if (event.key.code == sf::Keyboard::Left)
            leftKeyDown = false;
        if (event.key.code == sf::Keyboard::Right)
            rightKeyDown = false;
        if (event.key.code == sf::Keyboard::Enter)
            enterKeyDown = false;
        if (event.key.code == sf::Keyboard::Escape)
            escKeyDown = false;
        if (event.key.code == sf::Keyboard::Up)
            upKeyDown = false;
        if (event.key.code == sf::Keyboard::Down)
            downKeyDown = false;
        if (event.key.code == sf::Keyboard::I)
            inventoryKeyDown = false;
    }
}

// Handles Main Menu navigation and actions (Start / Load / Exit)
void InputManager::handleMainMenuInput(const sf::Event &event,
                                       Stack<GameStateData> &stateStack,
                                       bool &requestExit,
                                       SaveGameManager *saveManager)
{
    if (stateStack.isEmpty())
        return;

    GameStateData &mainMenuState = stateStack.top();

    if (event.key.code == sf::Keyboard::Up && !upKeyDown)
    {
        // Move selection up (wrap 0..2)
        mainMenuState.selectedMenuOption--;
        if (mainMenuState.selectedMenuOption < 0)
            mainMenuState.selectedMenuOption = 2;
        upKeyDown = true;
    }
    else if (event.key.code == sf::Keyboard::Down && !downKeyDown)
    {
        // Move selection down (wrap 0..2)
        mainMenuState.selectedMenuOption++;
        if (mainMenuState.selectedMenuOption > 2)
            mainMenuState.selectedMenuOption = 0;
        downKeyDown = true;
    }
    else if (event.key.code == sf::Keyboard::Enter && !enterKeyDown)
    {
        enterKeyDown = true;

        if (mainMenuState.selectedMenuOption == 0)
        {
            // Start — replace with Playing
            stateStack.pop();
            stateStack.push(GameStateData(GameStateType::Playing));
        }
        else if (mainMenuState.selectedMenuOption == 1)
        {
            // Load — push LoadGameMenu, default to first existing slot
            GameStateData loadMenuState(GameStateType::LoadGameMenu);
            loadMenuState.selectedMenuOption = 3; // Default to 'Back'

            if (saveManager)
            {
                if (saveManager->saveExists("savegame1"))
                    loadMenuState.selectedMenuOption = 0;
                else if (saveManager->saveExists("savegame2"))
                    loadMenuState.selectedMenuOption = 1;
                else if (saveManager->saveExists("savegame3"))
                    loadMenuState.selectedMenuOption = 2;
            }

            stateStack.push(loadMenuState);
        }
        else if (mainMenuState.selectedMenuOption == 2)
        {
            // Exit — flag for application shutdown
            requestExit = true;
        }
    }
}

// Handles in-game quick actions (Pause, Skill Tree)
void InputManager::handleGameInput(const sf::Event &event,
                                   Stack<GameStateData> &stateStack,
                                   bool gameOver,
                                   KeyBindingManager *keyManager)
{
    // Pause (Esc) if not in game-over state
    if (event.key.code == sf::Keyboard::Escape && !escKeyDown && !gameOver)
    {
        stateStack.push(GameStateData(GameStateType::Paused));
        escKeyDown = true;
    }

    // Open Skill Tree (T by default, or bound key)
    sf::Keyboard::Key skillTreeKey = sf::Keyboard::T;
    if (keyManager)
    {
        skillTreeKey = keyManager->getKeyForAction(GameAction::OPEN_SKILL_TREE);
    }

    if (event.key.code == skillTreeKey && !gameOver)
    {
        stateStack.push(GameStateData(GameStateType::SkillTreeScreen));
    }
}

// Handles Pause Menu (Resume, Save, Combat Log, Settings, Restart, Quit)
void InputManager::handlePauseMenuInput(const sf::Event &event,
                                        Stack<GameStateData> &stateStack,
                                        SinglyLinkedList<std::string> &combatLog,
                                        int &combatLogCurrentNode,
                                        int &combatLogTraversalCount,
                                        int &combatLogDeleteCount)
{
    if (stateStack.isEmpty())
        return;

    GameStateData &pauseState = stateStack.top();

    if (event.key.code == sf::Keyboard::Up && !upKeyDown)
    {
        // Up (wrap 0..5)
        pauseState.selectedMenuOption--;
        if (pauseState.selectedMenuOption < 0)
            pauseState.selectedMenuOption = 5;
        upKeyDown = true;
    }
    else if (event.key.code == sf::Keyboard::Down && !downKeyDown)
    {
        // Down (wrap 0..5)
        pauseState.selectedMenuOption++;
        if (pauseState.selectedMenuOption > 5)
            pauseState.selectedMenuOption = 0;
        downKeyDown = true;
    }
    else if (event.key.code == sf::Keyboard::Enter && !enterKeyDown)
    {
        // Execute selected action
        enterKeyDown = true;

        switch (pauseState.selectedMenuOption)
        {
        case 0: // Resume
            stateStack.pop();
            break;

        case 1: // Save Game
            stateStack.push(GameStateData(GameStateType::SaveGameMenu));
            break;

        case 2: // Combat Log
            combatLogCurrentNode = 0;
            combatLogTraversalCount = 0;
            combatLogDeleteCount = 0;
            stateStack.push(GameStateData(GameStateType::CombatLogScreen));
            break;

        case 3: // Settings
            stateStack.push(GameStateData(GameStateType::SettingsMenu));
            break;

        case 4: // Restart (confirm)
            stateStack.push(GameStateData(GameStateType::ConfirmRestart));
            break;

        case 5: // Quit to Menu (confirm)
            stateStack.push(GameStateData(GameStateType::ConfirmQuitToMenu));
            break;
        }
    }
    else if (event.key.code == sf::Keyboard::Escape && !escKeyDown)
    {
        // Escape — resume game
        stateStack.pop();
        escKeyDown = true;
    }
}

// Handles overlay-like screens (Stats, Inventory, Skill Tree, Help)
void InputManager::handleSimpleScreenInput(const sf::Event &event,
                                           Stack<GameStateData> &stateStack,
                                           Player *player,
                                           SinglyLinkedList<std::string> &combatLog,
                                           UISystem *uiSystem,
                                           KeyBindingManager *keyManager)
{
    if (stateStack.isEmpty())
        return;

    GameStateData &currentStateData = stateStack.top();
    GameStateType current = currentStateData.type;

    // Special handling for Skill Tree: grid nav + upgrades + branch reset
    if (current == GameStateType::SkillTreeScreen && player && uiSystem)
    {
        // Bound key for toggling Skill Tree
        sf::Keyboard::Key skillTreeKey = sf::Keyboard::T;
        if (keyManager)
        {
            skillTreeKey = keyManager->getKeyForAction(GameAction::OPEN_SKILL_TREE);
        }

        // Close Skill Tree with Esc or toggle key
        if ((event.key.code == sf::Keyboard::Escape || event.key.code == skillTreeKey) && !escKeyDown)
        {
            stateStack.pop();
            escKeyDown = true;
            return;
        }

        // Navigate selection within skill grid
        if (event.key.code == sf::Keyboard::Up && !upKeyDown)
        {
            // Move up between rows
            if (currentStateData.selectedSkillIndex >= 3 && currentStateData.selectedSkillIndex <= 5)
                currentStateData.selectedSkillIndex = 1;
            else if (currentStateData.selectedSkillIndex >= 6 && currentStateData.selectedSkillIndex <= 8)
                currentStateData.selectedSkillIndex = 2;
            else if (currentStateData.selectedSkillIndex == 1 || currentStateData.selectedSkillIndex == 2)
                currentStateData.selectedSkillIndex = 0;
            upKeyDown = true;
        }
        else if (event.key.code == sf::Keyboard::Down && !downKeyDown)
        {
            // Move down between rows
            if (currentStateData.selectedSkillIndex == 0)
                currentStateData.selectedSkillIndex = 1;
            else if (currentStateData.selectedSkillIndex == 1)
                currentStateData.selectedSkillIndex = 4;
            else if (currentStateData.selectedSkillIndex == 2)
                currentStateData.selectedSkillIndex = 7;
            downKeyDown = true;
        }
        else if (event.key.code == sf::Keyboard::Left && !leftKeyDown)
        {
            // Move left within a row
            if (currentStateData.selectedSkillIndex == 2)
                currentStateData.selectedSkillIndex = 1;
            else if (currentStateData.selectedSkillIndex == 4)
                currentStateData.selectedSkillIndex = 3;
            else if (currentStateData.selectedSkillIndex == 5)
                currentStateData.selectedSkillIndex = 4;
            else if (currentStateData.selectedSkillIndex == 6)
                currentStateData.selectedSkillIndex = 5;
            else if (currentStateData.selectedSkillIndex == 7)
                currentStateData.selectedSkillIndex = 6;
            else if (currentStateData.selectedSkillIndex == 8)
                currentStateData.selectedSkillIndex = 7;
            leftKeyDown = true;
        }
        else if (event.key.code == sf::Keyboard::Right && !rightKeyDown)
        {
            // Move right within a row
            if (currentStateData.selectedSkillIndex == 1)
                currentStateData.selectedSkillIndex = 2;
            else if (currentStateData.selectedSkillIndex == 3)
                currentStateData.selectedSkillIndex = 4;
            else if (currentStateData.selectedSkillIndex == 4)
                currentStateData.selectedSkillIndex = 5;
            else if (currentStateData.selectedSkillIndex == 5)
                currentStateData.selectedSkillIndex = 6;
            else if (currentStateData.selectedSkillIndex == 6)
                currentStateData.selectedSkillIndex = 7;
            else if (currentStateData.selectedSkillIndex == 7)
                currentStateData.selectedSkillIndex = 8;
            rightKeyDown = true;
        }
        else if (event.key.code == sf::Keyboard::Enter && !enterKeyDown)
        {
            // Try to upgrade / unlock selected skill
            enterKeyDown = true;

            SkillType selectedSkill;
            bool validSelection = true;

            switch (currentStateData.selectedSkillIndex)
            {
            case 0:
                // Base special attack already unlocked
                uiSystem->showToast("Special Attack is already unlocked!", sf::Color::Yellow);
                validSelection = false;
                break;
            case 1:
                selectedSkill = SkillType::FIRE_PROJECTILE;
                break;
            case 2:
                selectedSkill = SkillType::ICE_PROJECTILE;
                break;
            case 3:
                selectedSkill = SkillType::FIRE_DAMAGE_BOOST;
                break;
            case 4:
                selectedSkill = SkillType::FIRE_CHARGE_BOOST;
                break;
            case 5:
                selectedSkill = SkillType::FIRE_STACK_BOOST;
                break;
            case 6:
                selectedSkill = SkillType::ICE_DAMAGE_BOOST;
                break;
            case 7:
                selectedSkill = SkillType::ICE_CHARGE_BOOST;
                break;
            case 8:
                selectedSkill = SkillType::ICE_STACK_BOOST;
                break;
            default:
                validSelection = false;
                break;
            }

            if (validSelection && currentStateData.selectedSkillIndex != 0)
            {
                bool upgraded = player->getSkillTree().upgradeSkill(selectedSkill);
                if (upgraded)
                {
                    uiSystem->showToast("Skill upgraded successfully!", sf::Color::Green);
                    combatLog.pushBack("[Skill Tree] Upgraded skill (type: " + std::to_string(static_cast<int>(selectedSkill)) + ")");
                }
                else
                {
                    // Explain why upgrade failed (no points / prereq / max level)
                    if (player->getSkillTree().getSkillPoints() <= 0)
                        uiSystem->showToast("Not enough skill points!", sf::Color::Red);
                    else
                        uiSystem->showToast("Cannot upgrade: Prerequisites not met or max level", sf::Color::Red);
                }
            }
        }
        else if (event.key.code == sf::Keyboard::Num1)
        {
            // Reset Fire branch and refund points
            if (player->getSkillTree().hasFireProjectile())
            {
                int beforePoints = player->getSkillTree().getSkillPoints();
                player->getSkillTree().clearSkillBranch(SkillType::FIRE_PROJECTILE);
                int afterPoints = player->getSkillTree().getSkillPoints();
                int refunded = afterPoints - beforePoints;

                if (refunded > 0)
                {
                    uiSystem->showToast("Fire branch reset! Refunded " + std::to_string(refunded) + " skill points", sf::Color::Cyan);
                    combatLog.pushBack("[Skill Tree] Reset Fire branch - Refunded " + std::to_string(refunded) + " points");
                }
                else
                {
                    uiSystem->showToast("Fire branch has no upgrades to reset", sf::Color::Yellow);
                }
            }
            else
            {
                uiSystem->showToast("Fire Projectile not unlocked yet!", sf::Color::Red);
            }
        }
        else if (event.key.code == sf::Keyboard::Num2)
        {
            // Reset Ice branch and refund points
            if (player->getSkillTree().hasIceProjectile())
            {
                int beforePoints = player->getSkillTree().getSkillPoints();
                player->getSkillTree().clearSkillBranch(SkillType::ICE_PROJECTILE);
                int afterPoints = player->getSkillTree().getSkillPoints();
                int refunded = afterPoints - beforePoints;

                if (refunded > 0)
                {
                    uiSystem->showToast("Ice branch reset! Refunded " + std::to_string(refunded) + " skill points", sf::Color::Cyan);
                    combatLog.pushBack("[Skill Tree] Reset Ice branch - Refunded " + std::to_string(refunded) + " points");
                }
                else
                {
                    uiSystem->showToast("Ice branch has no upgrades to reset", sf::Color::Yellow);
                }
            }
            else
            {
                uiSystem->showToast("Ice Projectile not unlocked yet!", sf::Color::Red);
            }
        }

        return; // Done handling Skill Tree
    }

    // For other “simple” screens: Esc to close
    if (event.key.code == sf::Keyboard::Escape && !escKeyDown)
    {
        stateStack.pop();
        escKeyDown = true;
    }
}

// Handles Combat Log navigation, deletion, and exit
void InputManager::handleCombatLogInput(const sf::Event &event,
                                        Stack<GameStateData> &stateStack,
                                        SinglyLinkedList<std::string> &combatLog,
                                        int &combatLogCurrentNode,
                                        int &combatLogTraversalCount,
                                        int &combatLogDeleteCount)
{
    if (event.key.code == sf::Keyboard::Escape && !escKeyDown)
    {
        // Close Combat Log
        stateStack.pop();
        escKeyDown = true;
        combatLogCurrentNode = 0;
    }
    else if (event.key.code == sf::Keyboard::Right && !rightKeyDown)
    {
        // Move to next entry
        if (combatLogCurrentNode < combatLog.size() - 1)
        {
            combatLogCurrentNode++;
            combatLogTraversalCount++;
        }
        rightKeyDown = true;
    }
    else if (event.key.code == sf::Keyboard::Left && !leftKeyDown)
    {
        // Left reserved (no-op here)
        leftKeyDown = true;
    }
    else if (event.key.code == sf::Keyboard::Up && !upKeyDown)
    {
        // Jump to first entry
        combatLogCurrentNode = 0;
        combatLogTraversalCount++;
        upKeyDown = true;
    }
    else if (event.key.code == sf::Keyboard::Delete)
    {
        // Delete current entry by index
        if (combatLog.size() > 0)
        {
            int targetIndex = combatLogCurrentNode;
            int currentIndex = 0;

            combatLog.removeIf([&targetIndex, &currentIndex](const std::string &)
                               {
                                   bool shouldRemove = (currentIndex == targetIndex);
                                   currentIndex++;
                                   return shouldRemove; });

            combatLogDeleteCount++;

            // Clamp selection after deletion
            if (combatLogCurrentNode >= combatLog.size() && combatLog.size() > 0)
            {
                combatLogCurrentNode = combatLog.size() - 1;
            }
            else if (combatLog.size() == 0)
            {
                combatLogCurrentNode = 0;
            }
        }
    }
}

// Handles Settings Menu navigation and submenus
void InputManager::handleSettingsMenuInput(const sf::Event &event,
                                           Stack<GameStateData> &stateStack)
{
    if (stateStack.isEmpty())
        return;

    GameStateData &settingsState = stateStack.top();

    if (event.key.code == sf::Keyboard::Up && !upKeyDown)
    {
        // Up (wrap 0..3)
        settingsState.selectedMenuOption--;
        if (settingsState.selectedMenuOption < 0)
            settingsState.selectedMenuOption = 3;
        upKeyDown = true;
    }
    else if (event.key.code == sf::Keyboard::Down && !downKeyDown)
    {
        // Down (wrap 0..3)
        settingsState.selectedMenuOption++;
        if (settingsState.selectedMenuOption > 3)
            settingsState.selectedMenuOption = 0;
        downKeyDown = true;
    }
    else if (event.key.code == sf::Keyboard::Enter && !enterKeyDown)
    {
        // Activate selected submenu / back
        enterKeyDown = true;

        switch (settingsState.selectedMenuOption)
        {
        case 0:
            stateStack.push(GameStateData(GameStateType::AudioSettings));
            break;

        case 1:
            stateStack.push(GameStateData(GameStateType::ControlsMenu));
            break;

        case 2:
            stateStack.push(GameStateData(GameStateType::HelpScreen));
            break;

        case 3:
            stateStack.pop();
            break;
        }
    }
    else if (event.key.code == sf::Keyboard::Escape && !escKeyDown)
    {
        // Back to previous
        stateStack.pop();
        escKeyDown = true;
    }
}

// Handles Controls Menu navigation and the key-rebinding workflow
void InputManager::handleControlsMenuInput(const sf::Event &event,
                                           Stack<GameStateData> &stateStack,
                                           KeyBindingManager *keyManager,
                                           UISystem *uiSystem)
{
    if (stateStack.isEmpty() || !keyManager)
        return;

    GameStateData &controlsState = stateStack.top();

    // If currently waiting for a new key input to finish rebinding
    if (controlsState.awaitingKeyPress)
    {
        if (event.key.code == sf::Keyboard::Escape)
        {
            // Cancel rebinding
            controlsState.awaitingKeyPress = false;
            controlsState.rebindingAction = -1;
            if (uiSystem)
                uiSystem->showToast("Rebinding cancelled", sf::Color::Yellow);
        }
        else
        {
            // Attempt to bind the pressed key
            GameAction action = static_cast<GameAction>(controlsState.rebindingAction);

            GameAction conflictAction;
            if (keyManager->isKeyAlreadyBound(event.key.code, conflictAction) && conflictAction != action)
            {
                // Key already bound to another action
                std::string conflictName = KeyBindingManager::getActionName(conflictAction);
                if (uiSystem)
                    uiSystem->showToast("Key already bound to: " + conflictName, sf::Color::Red);
            }
            else
            {
                // Commit new binding
                if (keyManager->rebindAction(action, event.key.code))
                {
                    keyManager->saveBindings("keybindings.cfg");
                    if (uiSystem)
                        uiSystem->showToast("Key rebound successfully!", sf::Color::Green);
                }
                else
                {
                    if (uiSystem)
                        uiSystem->showToast("Failed to rebind key", sf::Color::Red);
                }
            }

            controlsState.awaitingKeyPress = false;
            controlsState.rebindingAction = -1;
        }
        return;
    }

    // Controls menu options: 0..8 actions, 9 = Reset, 10 = Back
    const int numActions = 9;
    const int resetOption = numActions;
    const int backOption = numActions + 1;
    const int maxOption = backOption;

    if (event.key.code == sf::Keyboard::Up && !upKeyDown)
    {
        // Up (wrap 0..10)
        controlsState.selectedMenuOption--;
        if (controlsState.selectedMenuOption < 0)
            controlsState.selectedMenuOption = maxOption;
        upKeyDown = true;
    }
    else if (event.key.code == sf::Keyboard::Down && !downKeyDown)
    {
        // Down (wrap 0..10)
        controlsState.selectedMenuOption++;
        if (controlsState.selectedMenuOption > maxOption)
            controlsState.selectedMenuOption = 0;
        downKeyDown = true;
    }
    else if (event.key.code == sf::Keyboard::Enter && !enterKeyDown)
    {
        // Activate selection
        enterKeyDown = true;

        if (controlsState.selectedMenuOption < numActions)
        {
            // Begin rebinding for selected action
            const GameAction actionMapping[] = {
                GameAction::MOVE_LEFT,
                GameAction::MOVE_RIGHT,
                GameAction::JUMP,
                GameAction::SHOOT_PROJECTILE,
                GameAction::SWITCH_PROJECTILE,
                GameAction::MELEE_ATTACK,
                GameAction::SPECIAL_ATTACK,
                GameAction::USE_POTION,
                GameAction::OPEN_SKILL_TREE};

            controlsState.awaitingKeyPress = true;
            controlsState.rebindingAction = static_cast<int>(actionMapping[controlsState.selectedMenuOption]);
        }
        else if (controlsState.selectedMenuOption == resetOption)
        {
            // Reset all to defaults
            keyManager->resetToDefaults();
            keyManager->saveBindings("keybindings.cfg");
            if (uiSystem)
                uiSystem->showToast("Controls reset to defaults", sf::Color::Green);
        }
        else if (controlsState.selectedMenuOption == backOption)
        {
            // Back to previous menu
            stateStack.pop();
        }
    }
    else if (event.key.code == sf::Keyboard::Escape && !escKeyDown)
    {
        // Close controls menu
        stateStack.pop();
        escKeyDown = true;
    }
}

// Handles Audio Settings (volume sliders, reset, back)
void InputManager::handleAudioSettingsInput(const sf::Event &event,
                                            Stack<GameStateData> &stateStack,
                                            ResourceManager &resourceManager)
{
    if (stateStack.isEmpty())
        return;

    GameStateData &audioState = stateStack.top();

    if (event.key.code == sf::Keyboard::Up && !upKeyDown)
    {
        // Up (wrap 0..3)
        audioState.selectedMenuOption--;
        if (audioState.selectedMenuOption < 0)
            audioState.selectedMenuOption = 3;
        upKeyDown = true;
    }
    else if (event.key.code == sf::Keyboard::Down && !downKeyDown)
    {
        // Down (wrap 0..3)
        audioState.selectedMenuOption++;
        if (audioState.selectedMenuOption > 3)
            audioState.selectedMenuOption = 0;
        downKeyDown = true;
    }
    else if (event.key.code == sf::Keyboard::Left && !leftKeyDown)
    {
        // Decrease selected volume (Music or SFX)
        leftKeyDown = true;

        switch (audioState.selectedMenuOption)
        {
        case 0: // Music -
            audioState.musicVolume = std::max(0.0f, audioState.musicVolume - 5.0f);
            ResourceManager::getInstance().setMusicVolume(audioState.musicVolume);
            break;

        case 1: // SFX -
            audioState.sfxVolume = std::max(0.0f, audioState.sfxVolume - 5.0f);
            ResourceManager::getInstance().setSFXVolume(audioState.sfxVolume);
            break;
        }
    }
    else if (event.key.code == sf::Keyboard::Right && !rightKeyDown)
    {
        // Increase selected volume (Music or SFX)
        rightKeyDown = true;

        switch (audioState.selectedMenuOption)
        {
        case 0: // Music +
            audioState.musicVolume = std::min(100.0f, audioState.musicVolume + 5.0f);
            ResourceManager::getInstance().setMusicVolume(audioState.musicVolume);
            break;

        case 1: // SFX +
            audioState.sfxVolume = std::min(100.0f, audioState.sfxVolume + 5.0f);
            ResourceManager::getInstance().setSFXVolume(audioState.sfxVolume);
            break;
        }
    }
    else if (event.key.code == sf::Keyboard::Enter && !enterKeyDown)
    {
        // Defaults or Back
        enterKeyDown = true;

        switch (audioState.selectedMenuOption)
        {
        case 2: // Reset volumes to defaults
            audioState.musicVolume = 80.0f;
            audioState.sfxVolume = 80.0f;
            ResourceManager::getInstance().setMusicVolume(audioState.musicVolume);
            ResourceManager::getInstance().setSFXVolume(audioState.sfxVolume);
            break;

        case 3: // Back
            stateStack.pop();
            break;
        }
    }
    else if (event.key.code == sf::Keyboard::Escape && !escKeyDown)
    {
        // Back with ESC
        stateStack.pop();
        escKeyDown = true;
    }
}

// Handles confirmation dialogs: Quit to Menu / Restart / Overwrite Save
void InputManager::handleConfirmDialogInput(const sf::Event &event,
                                            Stack<GameStateData> &stateStack,
                                            Player *player,
                                            class SaveGameManager *saveManager,
                                            UISystem *uiSystem)
{
    if (stateStack.isEmpty())
        return;

    GameStateData &confirmState = stateStack.top();

    if (event.key.code == sf::Keyboard::Left && !leftKeyDown)
    {
        // Select 'Yes'
        confirmState.selectedMenuOption = 0;
        leftKeyDown = true;
    }
    else if (event.key.code == sf::Keyboard::Right && !rightKeyDown)
    {
        // Select 'No'
        confirmState.selectedMenuOption = 1;
        rightKeyDown = true;
    }
    else if (event.key.code == sf::Keyboard::Enter && !enterKeyDown)
    {
        // Confirm selection
        enterKeyDown = true;

        if (confirmState.selectedMenuOption == 0)
        {
            // Yes branch
            if (confirmState.type == GameStateType::ConfirmQuitToMenu)
            {
                // Clear stack and go to Main Menu
                while (!stateStack.isEmpty())
                {
                    stateStack.pop();
                }
                stateStack.push(GameStateData(GameStateType::MainMenu));
            }
            else if (confirmState.type == GameStateType::ConfirmRestart)
            {
                // Close confirm and paused (two pops)
                stateStack.pop();
                stateStack.pop();
            }
            else if (confirmState.type == GameStateType::ConfirmOverwriteSave)
            {
                // Overwrite save specified by contextInfo
                if (saveManager && player && !confirmState.contextInfo.empty())
                {
                    std::string saveName = confirmState.contextInfo;
                    if (saveManager->saveGame(*player, saveName))
                    {
                        if (uiSystem)
                            uiSystem->showToast("Game saved successfully!", sf::Color::Green);
                    }
                    else
                    {
                        if (uiSystem)
                            uiSystem->showToast("Failed to save game!", sf::Color::Red);
                    }
                }
                // Pop confirm and the save menu beneath it
                stateStack.pop();
                if (!stateStack.isEmpty() && stateStack.top().type == GameStateType::SaveGameMenu)
                {
                    stateStack.pop();
                }
                return;
            }
        }
        else
        {
            // No — close dialog only
            stateStack.pop();
        }
    }
    else if (event.key.code == sf::Keyboard::Escape && !escKeyDown)
    {
        // ESC — close dialog
        stateStack.pop();
        escKeyDown = true;
    }
}

// Small getters for transient input flags (used by gameplay systems)
bool InputManager::isWalkLeftDown() const { return walkLeftDown; }
bool InputManager::isWalkRightDown() const { return walkRightDown; }
bool InputManager::isProjectileKeyPressed() const { return projectileKeyPressed; }
bool InputManager::isSwitchProjectileKeyPressed() const { return switchProjectileKeyPressed; }
bool InputManager::isAttack1KeyPressed() const { return attack1KeyPressed; }
bool InputManager::isAttack2KeyPressed() const { return attack2KeyPressed; }
bool InputManager::isInventoryKeyDown() const { return inventoryKeyDown; }

// Small setters for transient action flags (pressed this frame)
void InputManager::setProjectileKeyPressed(bool pressed) { projectileKeyPressed = pressed; }
void InputManager::setSwitchProjectileKeyPressed(bool pressed) { switchProjectileKeyPressed = pressed; }
void InputManager::setAttack1KeyPressed(bool pressed) { attack1KeyPressed = pressed; }
void InputManager::setAttack2KeyPressed(bool pressed) { attack2KeyPressed = pressed; }

// Handles Save Game menu (choose slot, confirm overwrite, or go back)
void InputManager::handleSaveGameMenuInput(const sf::Event &event,
                                           Stack<GameStateData> &stateStack,
                                           Player *player,
                                           class SaveGameManager *saveManager,
                                           UISystem *uiSystem)
{
    if (stateStack.isEmpty() || !player || !saveManager || !uiSystem)
        return;

    GameStateData &saveState = stateStack.top();

    if (event.key.code == sf::Keyboard::Up && !upKeyDown)
    {
        // Up (wrap 0..3)
        saveState.selectedMenuOption--;
        if (saveState.selectedMenuOption < 0)
            saveState.selectedMenuOption = 3;
        upKeyDown = true;
    }
    else if (event.key.code == sf::Keyboard::Down && !downKeyDown)
    {
        // Down (wrap 0..3)
        saveState.selectedMenuOption++;
        if (saveState.selectedMenuOption > 3)
            saveState.selectedMenuOption = 0;
        downKeyDown = true;
    }
    else if (event.key.code == sf::Keyboard::Enter && !enterKeyDown)
    {
        // Activate selection
        enterKeyDown = true;

        if (saveState.selectedMenuOption == 3)
        {
            // Back
            stateStack.pop();
        }
        else
        {
            // Save to selected slot (1..3)
            std::string saveName = "savegame" + std::to_string(saveState.selectedMenuOption + 1);

            // If slot exists, confirm overwrite
            if (saveManager->saveExists(saveName))
            {
                GameStateData confirmState(GameStateType::ConfirmOverwriteSave);
                confirmState.selectedMenuOption = 1; // default to 'No'
                confirmState.contextInfo = saveName; // pass target slot name
                stateStack.push(confirmState);
            }
            else
            {
                // Save immediately
                if (saveManager->saveGame(*player, saveName))
                {
                    uiSystem->showToast("Game saved successfully!", sf::Color::Green);
                }
                else
                {
                    uiSystem->showToast("Failed to save game!", sf::Color::Red);
                }
                stateStack.pop(); // Close Save menu
            }
        }
    }
    else if (event.key.code == sf::Keyboard::Escape && !escKeyDown)
    {
        // ESC — back
        stateStack.pop();
        escKeyDown = true;
    }
}

// Handles Load Game menu (choose an existing slot or back)
void InputManager::handleLoadGameMenuInput(const sf::Event &event,
                                           Stack<GameStateData> &stateStack,
                                           Player *player,
                                           class SaveGameManager *saveManager,
                                           UISystem *uiSystem)
{
    if (stateStack.isEmpty() || !player || !saveManager || !uiSystem)
        return;

    GameStateData &loadState = stateStack.top();

    // Availability for 3 slots + Back (always available)
    bool slotAvailable[4] = {false, false, false, true};
    slotAvailable[0] = saveManager->saveExists("savegame1");
    slotAvailable[1] = saveManager->saveExists("savegame2");
    slotAvailable[2] = saveManager->saveExists("savegame3");

    if (event.key.code == sf::Keyboard::Up && !upKeyDown)
    {
        // Move to previous available option (wrap)
        int startOption = loadState.selectedMenuOption;
        do
        {
            loadState.selectedMenuOption--;
            if (loadState.selectedMenuOption < 0)
                loadState.selectedMenuOption = 3;

            if (loadState.selectedMenuOption == startOption)
                break; // nothing else available
        } while (!slotAvailable[loadState.selectedMenuOption]);

        upKeyDown = true;
    }
    else if (event.key.code == sf::Keyboard::Down && !downKeyDown)
    {
        // Move to next available option (wrap)
        int startOption = loadState.selectedMenuOption;
        do
        {
            loadState.selectedMenuOption++;
            if (loadState.selectedMenuOption > 3)
                loadState.selectedMenuOption = 0;

            if (loadState.selectedMenuOption == startOption)
                break; // nothing else available
        } while (!slotAvailable[loadState.selectedMenuOption]);

        downKeyDown = true;
    }
    else if (event.key.code == sf::Keyboard::Enter && !enterKeyDown)
    {
        // Activate selection
        enterKeyDown = true;

        if (loadState.selectedMenuOption == 3)
        {
            // Back
            stateStack.pop();
        }
        else if (slotAvailable[loadState.selectedMenuOption])
        {
            // Load selected save
            std::string saveName = "savegame" + std::to_string(loadState.selectedMenuOption + 1);
            if (saveManager->loadGame(*player, saveName))
            {
                uiSystem->showToast("Game loaded successfully!", sf::Color::Green);

                // Clear all states and go back to Playing
                while (!stateStack.isEmpty())
                {
                    stateStack.pop();
                }
                stateStack.push(GameStateData(GameStateType::Playing));
            }
            else
            {
                uiSystem->showToast("Failed to load game!", sf::Color::Red);
            }
        }
    }
    else if (event.key.code == sf::Keyboard::Escape && !escKeyDown)
    {
        // ESC — back
        stateStack.pop();
        escKeyDown = true;
    }
}