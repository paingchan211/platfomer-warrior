#pragma once

#include <SFML/System/Time.hpp>
#include <string>

// Defines all possible game states
enum class GameStateType
{
    MainMenu,
    LoadGameMenu,
    SaveGameMenu,
    Playing,
    Paused,
    GameOver,
    BossDeathSlowMotion,
    SettingsMenu,
    AudioSettings,
    ControlsMenu,
    PlayerStatsScreen,
    InventoryOnlyScreen,
    SkillTreeScreen,
    HelpScreen,
    CombatLogScreen,
    DebugMenu,
    ConfirmQuitToMenu,
    ConfirmRestart,
    ConfirmOverwriteSave
};

class GameStateBehavior;

// Returns a behavior instance for the requested state type
const GameStateBehavior &getGameStateBehavior(GameStateType type);

// Utility to convert a state type into a readable string for logging/debug
const char *toString(GameStateType type);

// Holds detailed state information for current game mode
struct GameStateData
{
    GameStateData(); // Default constructor (Playing)
    explicit GameStateData(GameStateType stateType);

    GameStateType type;      // Current state type
    sf::Time timeEntered;    // Timestamp when entered this state
    bool pausesGameLogic;    // Whether game logic is paused
    bool pausesRendering;    // Whether rendering is paused
    bool allowsInput;        // Whether player input is allowed
    std::string contextInfo; // Optional context information

    // Menu and skill state
    int selectedMenuOption;
    int selectedSkillIndex;
    int selectedDebugOption;

    // Audio settings
    float musicVolume;
    float sfxVolume;

    // Control remapping state
    bool awaitingKeyPress;
    int rebindingAction;

    // Returns current behavior (safe because behaviors are singletons)
    const GameStateBehavior &behavior() const;

private:
    const GameStateBehavior *stateBehavior; // Behavior implementing the pattern

    void initializeDefaults();
    void applyBehavior(GameStateType stateType);
};

// Base interface for all concrete states
class GameStateBehavior
{
public:
    virtual ~GameStateBehavior() = default;

    // Every state exposes its type for debugging and comparisons
    virtual GameStateType getType() const = 0;

    // Flags that describe how this state affects the simulation
    virtual bool pausesGameLogic() const = 0;
    virtual bool pausesRendering() const = 0;
    virtual bool allowsInput() const = 0;

    // Lifecycle hooks that derived states may override if they need work
    virtual void onEnter(GameStateData &state) const;
    virtual void onExit(GameStateData &state) const;
};
