#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

class Player;
class Enemy;
class GameWorld;

// Manages special combat effects like Rage Mode, Meteor Fury, and Boss Rage
class GameMaster
{
public:
    GameMaster();  // Constructor
    ~GameMaster(); // Destructor

    bool initialize(); // Loads sounds and initializes variables

    void update(float dt, const Player &player, const GameWorld &gameWorld);                // Updates all active modes
    void render(sf::RenderWindow &window, const sf::View &worldView, const Player *player, const sf::Font &font); // Renders rage overlay + events
    void reset();                                                                           // Resets all states
    void stopAllSounds();                                                                   // Stops playing sounds

    bool isRageModeActive() const;           // Checks if rage mode is active
    float getPlayerDamageMultiplier() const; // Returns player’s damage multiplier
    bool isMeteorFuryActive() const;         // Checks if meteor fury is active
    float getMeteorSpawnInterval() const;    // Returns current meteor spawn interval
    bool isBossRageModeActive() const;       // Checks if boss rage mode is active
    float getBossDamageMultiplier() const;   // Returns boss's damage multiplier
    bool isSandStormActive() const;          // Checks if sand storm is currently restricting vision/movement

private:
    // Rage mode functions
    void updateRageMode(float dt, const Player &player, const GameWorld &gameWorld);
    void activateRageMode();
    void deactivateRageMode();
    int countNearbyEnemies(const Player &player, const GameWorld &gameWorld) const;

    // Meteor fury functions
    void updateMeteorFury(float dt, const GameWorld &gameWorld);
    void activateMeteorFury();
    void deactivateMeteorFury();

    // Boss rage mode functions
    void updateBossRageMode(float dt, const GameWorld &gameWorld);
    void activateBossRageMode();
    void deactivateBossRageMode();

    // Sand storm helpers
    void updateSandStorm(float dt, const GameWorld &gameWorld);
    void renderSandStorm(sf::RenderWindow &window, const sf::Vector2f &focusPoint, const sf::Font &font);
    void ensureSandStormTexture(const sf::Vector2u &windowSize);
    int getDefeatedEnemyCount(const GameWorld &gameWorld) const;

    // Rage mode variables
    bool rageModeActive;
    float rageModeDuration;
    float rageModeTimer;
    float rageModeCooldown;
    float rageModeCooldownTimer;
    float rageDamageMultiplier;

    // Visual overlay variables
    sf::RectangleShape screenOverlay;
    float overlayAlpha;
    float overlayPulseTimer;

    // Audio
    sf::SoundBuffer rageSoundBuffer;
    sf::Sound rageSound;
    float rageSoundVolume;

    // Meteor fury variables
    bool meteorFuryActive;
    bool meteorFuryTriggeredOnce;
    float meteorSpawnInterval;
    float normalMeteorInterval;
    float furyMeteorInterval;

    // Boss rage mode variables
    bool bossRageModeActive;
    bool bossRageModeTriggeredOnce;
    float bossDamageMultiplier;

    // Sand storm variables
    bool sandStormTriggerAnnounced;
    bool sandStormWarningActive;
    bool sandStormInProgress;
    float sandStormWarningDuration;
    float sandStormWarningTimer;
    float sandStormTimer;
    float sandStormFadeDuration;
    float sandStormActiveDuration;
    float sandStormIntensity;
    float sandStormVisibilityRadius;
    float sandStormVisibilityMinRadius;
    float sandStormVisibilityMaxRadius;
    float sandStormFogMaxAlpha;
    int sandStormKillsRequired;

    sf::RenderTexture sandStormRenderTexture;
    sf::Sprite sandStormSprite;
    sf::Vector2u sandStormMaskSize;
};
