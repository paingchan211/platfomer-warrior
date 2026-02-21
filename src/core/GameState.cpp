#include "GameState.h"
#include "Stack.h"

#include <iostream>
#include <stdexcept>

void StackDebugLogger<GameStateData>::pushed(const GameStateData &state)
{
    if (ENABLE_STACK_STDOUT)
        std::cout << "[Stack] " << toString(state.type) << " pushed into the StateStack" << std::endl;
}

void StackDebugLogger<GameStateData>::popped(const GameStateData &state)
{
    if (ENABLE_STACK_STDOUT)
        std::cout << "[Stack] " << toString(state.type) << " popped out of the StateStack" << std::endl;
}

namespace
{
    // Simple concrete state that only needs flag configuration
    class SimpleGameStateBehavior : public GameStateBehavior
    {
    public:
        SimpleGameStateBehavior(GameStateType state,
                                bool pauseLogic,
                                bool pauseRender,
                                bool allowInput)
            : stateType(state),
              pausesLogic(pauseLogic),
              pausesRender(pauseRender),
              inputAllowed(allowInput)
        {
        }

        GameStateType getType() const override { return stateType; }
        bool pausesGameLogic() const override { return pausesLogic; }
        bool pausesRendering() const override { return pausesRender; }
        bool allowsInput() const override { return inputAllowed; }

    private:
        GameStateType stateType;
        bool pausesLogic;
        bool pausesRender;
        bool inputAllowed;
    };

    // Convenience helper for grouped menu states
    const GameStateBehavior &menuState(GameStateType type)
    {
        static SimpleGameStateBehavior load(GameStateType::LoadGameMenu, true, false, true);
        static SimpleGameStateBehavior save(GameStateType::SaveGameMenu, true, false, true);
        static SimpleGameStateBehavior paused(GameStateType::Paused, true, false, true);
        static SimpleGameStateBehavior settings(GameStateType::SettingsMenu, true, false, true);
        static SimpleGameStateBehavior audio(GameStateType::AudioSettings, true, false, true);
        static SimpleGameStateBehavior controls(GameStateType::ControlsMenu, true, false, true);
        static SimpleGameStateBehavior playerStats(GameStateType::PlayerStatsScreen, true, false, true);
        static SimpleGameStateBehavior inventory(GameStateType::InventoryOnlyScreen, true, false, true);
        static SimpleGameStateBehavior skillTree(GameStateType::SkillTreeScreen, true, false, true);
        static SimpleGameStateBehavior help(GameStateType::HelpScreen, true, false, true);
        static SimpleGameStateBehavior combat(GameStateType::CombatLogScreen, true, false, true);
        static SimpleGameStateBehavior debugMenu(GameStateType::DebugMenu, true, false, true);
        static SimpleGameStateBehavior consoleDebug(GameStateType::ConsoleDebugMenu, true, false, true);
        static SimpleGameStateBehavior confirmQuit(GameStateType::ConfirmQuitToMenu, true, false, true);
        static SimpleGameStateBehavior confirmRestart(GameStateType::ConfirmRestart, true, false, true);
        static SimpleGameStateBehavior confirmOverwrite(GameStateType::ConfirmOverwriteSave, true, false, true);

        switch (type)
        {
        case GameStateType::LoadGameMenu:
            return load;
        case GameStateType::SaveGameMenu:
            return save;
        case GameStateType::Paused:
            return paused;
        case GameStateType::SettingsMenu:
            return settings;
        case GameStateType::AudioSettings:
            return audio;
        case GameStateType::ControlsMenu:
            return controls;
        case GameStateType::PlayerStatsScreen:
            return playerStats;
        case GameStateType::InventoryOnlyScreen:
            return inventory;
        case GameStateType::SkillTreeScreen:
            return skillTree;
        case GameStateType::HelpScreen:
            return help;
        case GameStateType::CombatLogScreen:
            return combat;
        case GameStateType::DebugMenu:
            return debugMenu;
        case GameStateType::ConsoleDebugMenu:
            return consoleDebug;
        case GameStateType::ConfirmQuitToMenu:
            return confirmQuit;
        case GameStateType::ConfirmRestart:
            return confirmRestart;
        case GameStateType::ConfirmOverwriteSave:
            return confirmOverwrite;
        default:
            throw std::logic_error("Unsupported menu state");
        }
    }
} // namespace

const GameStateBehavior &getGameStateBehavior(GameStateType type)
{
    switch (type)
    {
    case GameStateType::MainMenu:
    {
        static SimpleGameStateBehavior behavior(GameStateType::MainMenu, false, false, true);
        return behavior;
    }
    case GameStateType::Playing:
    {
        static SimpleGameStateBehavior behavior(GameStateType::Playing, false, false, true);
        return behavior;
    }
    case GameStateType::GameOver:
    {
        static SimpleGameStateBehavior behavior(GameStateType::GameOver, true, false, false);
        return behavior;
    }
    case GameStateType::BossDeathSlowMotion:
    {
        static SimpleGameStateBehavior behavior(GameStateType::BossDeathSlowMotion, false, false, false);
        return behavior;
    }
    case GameStateType::LoadGameMenu:
    case GameStateType::SaveGameMenu:
    case GameStateType::Paused:
    case GameStateType::SettingsMenu:
    case GameStateType::AudioSettings:
    case GameStateType::ControlsMenu:
    case GameStateType::PlayerStatsScreen:
    case GameStateType::InventoryOnlyScreen:
    case GameStateType::SkillTreeScreen:
    case GameStateType::HelpScreen:
    case GameStateType::CombatLogScreen:
    case GameStateType::DebugMenu:
    case GameStateType::ConsoleDebugMenu:
    case GameStateType::ConfirmQuitToMenu:
    case GameStateType::ConfirmRestart:
    case GameStateType::ConfirmOverwriteSave:
        return menuState(type);
    default:
        throw std::logic_error("Unknown GameStateType");
    }
}

const char *toString(GameStateType type)
{
    switch (type)
    {
    case GameStateType::MainMenu:
        return "MainMenu";
    case GameStateType::LoadGameMenu:
        return "LoadGameMenu";
    case GameStateType::SaveGameMenu:
        return "SaveGameMenu";
    case GameStateType::Playing:
        return "Playing";
    case GameStateType::Paused:
        return "Paused";
    case GameStateType::GameOver:
        return "GameOver";
    case GameStateType::BossDeathSlowMotion:
        return "BossDeathSlowMotion";
    case GameStateType::SettingsMenu:
        return "SettingsMenu";
    case GameStateType::AudioSettings:
        return "AudioSettings";
    case GameStateType::ControlsMenu:
        return "ControlsMenu";
    case GameStateType::PlayerStatsScreen:
        return "PlayerStatsScreen";
    case GameStateType::InventoryOnlyScreen:
        return "InventoryOnlyScreen";
    case GameStateType::SkillTreeScreen:
        return "SkillTreeScreen";
    case GameStateType::HelpScreen:
        return "HelpScreen";
    case GameStateType::CombatLogScreen:
        return "CombatLogScreen";
    case GameStateType::DebugMenu:
        return "DebugMenu";
    case GameStateType::ConsoleDebugMenu:
        return "ConsoleDebugMenu";
    case GameStateType::ConfirmQuitToMenu:
        return "ConfirmQuitToMenu";
    case GameStateType::ConfirmRestart:
        return "ConfirmRestart";
    case GameStateType::ConfirmOverwriteSave:
        return "ConfirmOverwriteSave";
    default:
        return "Unknown";
    }
}

// ---------- GameStateBehavior ----------

void GameStateBehavior::onEnter(GameStateData &) const {}

void GameStateBehavior::onExit(GameStateData &) const {}

// ---------- GameStateData ----------

GameStateData::GameStateData()
    : GameStateData(GameStateType::Playing)
{
}

GameStateData::GameStateData(GameStateType stateType)
{
    initializeDefaults();
    applyBehavior(stateType);
}

const GameStateBehavior &GameStateData::behavior() const
{
    if (!stateBehavior)
    {
        return getGameStateBehavior(GameStateType::Playing);
    }
    return *stateBehavior;
}

void GameStateData::initializeDefaults()
{
    type = GameStateType::Playing;
    timeEntered = sf::Time::Zero;
    pausesGameLogic = false;
    pausesRendering = false;
    allowsInput = true;
    contextInfo.clear();
    selectedMenuOption = 0;
    selectedSkillIndex = 1;
    selectedDebugOption = 0;
    musicVolume = 80.0f;
    sfxVolume = 80.0f;
    awaitingKeyPress = false;
    rebindingAction = -1;
    stateBehavior = nullptr;
}

void GameStateData::applyBehavior(GameStateType stateType)
{
    type = stateType;
    const GameStateBehavior &behaviorRef = getGameStateBehavior(stateType);
    stateBehavior = &behaviorRef;
    pausesGameLogic = behaviorRef.pausesGameLogic();
    pausesRendering = behaviorRef.pausesRendering();
    allowsInput = behaviorRef.allowsInput();
    behaviorRef.onEnter(*this);
}
