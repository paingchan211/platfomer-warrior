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
    ConfirmQuitToMenu,
    ConfirmRestart,
    ConfirmOverwriteSave
};

// Holds detailed state information for current game mode
struct GameStateData
{
    GameStateType type;      // Current state type
    sf::Time timeEntered;    // Timestamp when entered this state
    bool pausesGameLogic;    // Whether game logic is paused
    bool pausesRendering;    // Whether rendering is paused
    bool allowsInput;        // Whether player input is allowed
    std::string contextInfo; // Optional context information

    // Menu and skill state
    int selectedMenuOption;
    int selectedSkillIndex;

    // Audio settings
    float musicVolume;
    float sfxVolume;

    // Control remapping state
    bool awaitingKeyPress;
    int rebindingAction;

    // Default constructor (for playing state)
    GameStateData()
        : type(GameStateType::Playing),
          pausesGameLogic(false),
          pausesRendering(false),
          allowsInput(true),
          selectedMenuOption(0),
          selectedSkillIndex(1),
          musicVolume(80.0f),
          sfxVolume(80.0f),
          awaitingKeyPress(false),
          rebindingAction(-1)
    {
    }

    // Constructor that configures behavior depending on state type
    explicit GameStateData(GameStateType stateType)
        : type(stateType),
          pausesGameLogic(false),
          pausesRendering(false),
          allowsInput(true),
          selectedMenuOption(0),
          selectedSkillIndex(1),
          musicVolume(80.0f),
          sfxVolume(80.0f),
          awaitingKeyPress(false),
          rebindingAction(-1)
    {
        // Configure behavior based on specific state type
        switch (stateType)
        {
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
        case GameStateType::ConfirmQuitToMenu:
        case GameStateType::ConfirmRestart:
            pausesGameLogic = true;
            break;
        case GameStateType::BossDeathSlowMotion:
            pausesGameLogic = false;
            break;
        case GameStateType::GameOver:
            pausesGameLogic = true;
            break;
        default:
            break;
        }
    }
};
