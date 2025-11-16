#include "GameMaster.h"
#include "Player.h"
#include "Enemy.h"
#include "GameWorld.h"
#include "Constants.h"
#include "ResourceManager.h"
#include <algorithm>
#include <cmath>
#include <iostream>

// Constructor initializing all gameplay effect variables
GameMaster::GameMaster()
    : rageModeActive(false),
      rageModeDuration(RAGE_DURATION),
      rageModeTimer(0.0f),
      rageModeCooldown(RAGE_COOLDOWN),
      rageModeCooldownTimer(0.0f),
      rageDamageMultiplier(1.0f),
      overlayAlpha(0.0f),
      overlayPulseTimer(0.0f),
      rageSoundVolume(80.0f),
      meteorFuryActive(false),
      meteorFuryTriggeredOnce(false),
      meteorSpawnInterval(15.0f),
      normalMeteorInterval((METEOR_NORMAL_MIN_INTERVAL + METEOR_NORMAL_MAX_INTERVAL) / 2.0f),
      furyMeteorInterval((METEOR_FURY_MIN_INTERVAL + METEOR_FURY_MAX_INTERVAL) / 2.0f),
      bossRageModeActive(false),
      bossRageModeTriggeredOnce(false),
      bossDamageMultiplier(1.0f),
      sandStormTriggerAnnounced(false),
      sandStormWarningActive(false),
      sandStormInProgress(false),
      sandStormWarningDuration(3.0f),
      sandStormWarningTimer(0.0f),
      sandStormTimer(0.0f),
      sandStormFadeDuration(4.0f),
      sandStormActiveDuration(10.0f),
      sandStormIntensity(0.0f),
      sandStormVisibilityRadius(0.0f),
      sandStormVisibilityMinRadius(90.0f),
      sandStormVisibilityMaxRadius(220.0f),
      sandStormFogMaxAlpha(235.0f),
      sandStormKillsRequired(3),
      sandStormMaskSize(0, 0)
{
    // Initialize red screen overlay with zero transparency
    screenOverlay.setFillColor(sf::Color(255, 0, 0, 0));
    sandStormVisibilityRadius = sandStormVisibilityMaxRadius;
}

// Destructor
GameMaster::~GameMaster()
{
}

// Loads sounds and initializes variables
bool GameMaster::initialize()
{
    if (!rageSoundBuffer.loadFromFile("assets/sounds/rage.wav"))
    {
        std::cerr << "Warning: Failed to load rage sound: assets/sounds/rage.wav" << std::endl;
    }
    else
    {
        rageSound.setBuffer(rageSoundBuffer);
        rageSound.setVolume(rageSoundVolume);
    }

    return true;
}

// Updates all rage-related and meteor events
void GameMaster::update(float dt, const Player &player, const GameWorld &gameWorld)
{
    updateSandStorm(dt, gameWorld);
    updateRageMode(dt, player, gameWorld);
    updateMeteorFury(dt, gameWorld);
    updateBossRageMode(dt, gameWorld);
}

// Renders overlays for rage mode and the sand storm event
void GameMaster::render(sf::RenderWindow &window, const sf::View &worldView, const Player *player)
{
    bool drawRageOverlay = overlayAlpha > 0.0f;
    bool drawSandStorm = sandStormWarningActive || sandStormInProgress;

    if (!drawRageOverlay && !drawSandStorm)
    {
        return;
    }

    sf::View previousView = window.getView();
    sf::Vector2f windowSize(static_cast<float>(window.getSize().x),
                            static_cast<float>(window.getSize().y));

    // Calculate player screen position while still using world view
    sf::Vector2f sandStormFocus(windowSize.x / 2.0f, windowSize.y / 2.0f);
    if (player)
    {
        // Map player position to pixel coordinates relative to the world view
        sf::Vector2i pixelPos = window.mapCoordsToPixel(player->getPosition(), worldView);
        sandStormFocus = sf::Vector2f(static_cast<float>(pixelPos.x), static_cast<float>(pixelPos.y));

        // Slight offset so the visibility pocket aligns with the player's rendered torso
        const sf::Vector2f sandStormFocusOffset(100.f, 102.f);
        sandStormFocus += sandStormFocusOffset;
    }

    sf::View screenView(sf::FloatRect(0.f, 0.f, windowSize.x, windowSize.y));
    window.setView(screenView);

    if (drawRageOverlay)
    {
        screenOverlay.setSize(sf::Vector2f(window.getSize().x, window.getSize().y));
        screenOverlay.setPosition(0.0f, 0.0f);
        window.draw(screenOverlay);
    }

    if (drawSandStorm)
    {
        renderSandStorm(window, sandStormFocus);
    }

    window.setView(previousView);
}

// Resets all temporary game state effects
void GameMaster::reset()
{
    rageModeActive = false;
    rageModeTimer = 0.0f;
    rageModeCooldownTimer = 0.0f;
    rageDamageMultiplier = 1.0f;
    overlayAlpha = 0.0f;
    overlayPulseTimer = 0.0f;

    if (rageSound.getStatus() == sf::Sound::Playing)
    {
        rageSound.stop();
    }

    meteorFuryActive = false;
    meteorFuryTriggeredOnce = false;
    meteorSpawnInterval = normalMeteorInterval;

    bossRageModeActive = false;
    bossRageModeTriggeredOnce = false;
    bossDamageMultiplier = 1.0f;

    sandStormTriggerAnnounced = false;
    sandStormWarningActive = false;
    sandStormInProgress = false;
    sandStormWarningTimer = 0.0f;
    sandStormTimer = 0.0f;
    sandStormIntensity = 0.0f;
    sandStormVisibilityRadius = sandStormVisibilityMaxRadius;
}

// Stops all currently playing sounds
void GameMaster::stopAllSounds()
{
    if (rageSound.getStatus() == sf::Sound::Playing)
    {
        rageSound.stop();
    }
}

// Returns true if player rage mode is active
bool GameMaster::isRageModeActive() const
{
    return rageModeActive;
}

// Returns player damage multiplier (increased during rage)
float GameMaster::getPlayerDamageMultiplier() const
{
    return rageDamageMultiplier;
}

// Returns true if meteor fury mode is active
bool GameMaster::isMeteorFuryActive() const
{
    return meteorFuryActive;
}

// Returns current meteor spawn interval
float GameMaster::getMeteorSpawnInterval() const
{
    return meteorSpawnInterval;
}

// Handles player rage mode activation and cooldown
void GameMaster::updateRageMode(float dt, const Player &player, const GameWorld &gameWorld)
{
    if (rageModeCooldownTimer > 0.0f)
    {
        rageModeCooldownTimer -= dt;
    }

    if (rageModeActive)
    {
        rageModeTimer -= dt;
        if (rageModeTimer <= 0.0f)
        {
            deactivateRageMode();
        }
    }
    else
    {
        // Check activation conditions
        float hpPercentage = static_cast<float>(player.getHp()) / static_cast<float>(player.getMaxHp());
        bool lowHealth = (hpPercentage < RAGE_HP_THRESHOLD);

        int nearbyEnemies = countNearbyEnemies(player, gameWorld);
        bool enoughEnemies = (nearbyEnemies >= RAGE_MIN_ENEMIES);

        bool notOnCooldown = (rageModeCooldownTimer <= 0.0f);

        if (lowHealth && enoughEnemies && notOnCooldown)
        {
            activateRageMode();
        }
    }
}

// Activates player rage mode
void GameMaster::activateRageMode()
{
    rageModeActive = true;
    rageModeTimer = rageModeDuration;
    rageDamageMultiplier = 1.0f + RAGE_DAMAGE_BONUS;
}

// Deactivates player rage mode and starts cooldown
void GameMaster::deactivateRageMode()
{
    rageModeActive = false;
    rageModeCooldownTimer = rageModeCooldown;
    rageDamageMultiplier = 1.0f;

    if (rageSound.getStatus() == sf::Sound::Playing)
    {
        rageSound.stop();
    }

    std::cout << "Rage mode ended. Cooldown active." << std::endl;
}

// Counts enemies near the player for rage activation check
int GameMaster::countNearbyEnemies(const Player &player, const GameWorld &gameWorld) const
{
    int count = 0;
    sf::Vector2f playerPos = player.getPosition();

    const auto &enemies = gameWorld.getEnemies();
    size_t enemyCount = gameWorld.getEnemyCount();

    for (size_t i = 0; i < enemyCount; ++i)
    {
        if (enemies[i] && enemies[i]->isAlive())
        {
            sf::Vector2f enemyPos = enemies[i]->getPosition();
            float distance = std::sqrt(
                (playerPos.x - enemyPos.x) * (playerPos.x - enemyPos.x) +
                (playerPos.y - enemyPos.y) * (playerPos.y - enemyPos.y));

            if (distance <= RAGE_DETECTION_RANGE)
            {
                count++;
            }
        }
    }

    const Boss *boss = gameWorld.getBoss();
    if (boss && boss->isAlive())
    {
        sf::Vector2f bossPos = boss->getPosition();
        float distance = std::sqrt(
            (playerPos.x - bossPos.x) * (playerPos.x - bossPos.x) +
            (playerPos.y - bossPos.y) * (playerPos.y - bossPos.y));

        if (distance <= RAGE_DETECTION_RANGE)
        {
            count++;
        }
    }

    return count;
}

// Updates meteor fury mode based on boss health
void GameMaster::updateMeteorFury(float dt, const GameWorld &gameWorld)
{
    const Boss *boss = gameWorld.getBoss();

    if (boss && boss->isAlive())
    {
        float bossHpPercentage = static_cast<float>(boss->getHp()) / static_cast<float>(boss->getMaxHp());
        bool bossVeryLowHealth = (bossHpPercentage < 0.30f);

        if (bossVeryLowHealth && !meteorFuryActive)
        {
            activateMeteorFury();
            meteorFuryTriggeredOnce = true;
        }
        else if (!bossVeryLowHealth && meteorFuryActive && !meteorFuryTriggeredOnce)
        {
            deactivateMeteorFury();
        }
    }
    else if (meteorFuryActive)
    {
        deactivateMeteorFury();
    }
}

// Activates meteor fury mode (boss ability)
void GameMaster::activateMeteorFury()
{
    meteorFuryActive = true;
    meteorSpawnInterval = furyMeteorInterval;
}

// Deactivates meteor fury mode
void GameMaster::deactivateMeteorFury()
{
    meteorFuryActive = false;
    meteorSpawnInterval = normalMeteorInterval;

    std::cout << "Meteor fury subsided..." << std::endl;
}

// Updates boss rage mode based on low HP
void GameMaster::updateBossRageMode(float dt, const GameWorld &gameWorld)
{
    const Boss *boss = gameWorld.getBoss();

    if (boss && boss->isAlive())
    {
        float bossHpPercentage = static_cast<float>(boss->getHp()) / static_cast<float>(boss->getMaxHp());
        bool bossLowHealth = (bossHpPercentage < BOSS_RAGE_HP_THRESHOLD);

        if (bossLowHealth && !bossRageModeActive)
        {
            activateBossRageMode();
            bossRageModeTriggeredOnce = true;
        }
        else if (!bossLowHealth && bossRageModeActive && !bossRageModeTriggeredOnce)
        {
            deactivateBossRageMode();
        }
    }
    else if (bossRageModeActive)
    {
        deactivateBossRageMode();
    }
}

// Activates boss rage mode (increases damage)
void GameMaster::activateBossRageMode()
{
    bossRageModeActive = true;
    bossDamageMultiplier = 1.0f + BOSS_RAGE_DAMAGE_BONUS;

    if (rageSoundBuffer.getSampleCount() > 0)
    {
        rageSound.play();
    }
}

// Deactivates boss rage mode
void GameMaster::deactivateBossRageMode()
{
    bossRageModeActive = false;
    bossDamageMultiplier = 1.0f;
}

// Returns true if the boss rage mode is active
bool GameMaster::isBossRageModeActive() const
{
    return bossRageModeActive;
}

// Returns the boss’s current damage multiplier
float GameMaster::getBossDamageMultiplier() const
{
    return bossDamageMultiplier;
}

bool GameMaster::isSandStormActive() const
{
    return sandStormInProgress;
}

// Handles scheduling and timers for the sand storm event
void GameMaster::updateSandStorm(float dt, const GameWorld &gameWorld)
{
    if (!sandStormTriggerAnnounced)
    {
        int defeatedEnemies = getDefeatedEnemyCount(gameWorld);
        if (defeatedEnemies >= sandStormKillsRequired)
        {
            sandStormTriggerAnnounced = true;
            sandStormWarningActive = true;
            sandStormWarningTimer = sandStormWarningDuration;
        }
    }

    if (sandStormWarningActive)
    {
        sandStormWarningTimer = std::max(0.0f, sandStormWarningTimer - dt);
        if (sandStormWarningTimer <= 0.0f)
        {
            sandStormWarningActive = false;
            sandStormInProgress = true;
            sandStormTimer = 0.0f;
        }
    }

    if (sandStormInProgress)
    {
        sandStormTimer += dt;

        float fadeInEnd = sandStormFadeDuration;
        float activeEnd = fadeInEnd + sandStormActiveDuration;
        float fadeOutEnd = activeEnd + sandStormFadeDuration;

        if (sandStormTimer <= fadeInEnd)
        {
            sandStormIntensity = std::min(sandStormTimer / sandStormFadeDuration, 1.0f);
        }
        else if (sandStormTimer <= activeEnd)
        {
            sandStormIntensity = 1.0f;
        }
        else if (sandStormTimer <= fadeOutEnd)
        {
            float fadeProgress = (sandStormTimer - activeEnd) / sandStormFadeDuration;
            sandStormIntensity = std::max(0.0f, 1.0f - fadeProgress);
        }
        else
        {
            sandStormInProgress = false;
            sandStormIntensity = 0.0f;
            sandStormVisibilityRadius = sandStormVisibilityMaxRadius;
            return;
        }

        sandStormVisibilityRadius = sandStormVisibilityMaxRadius -
                                    (sandStormVisibilityMaxRadius - sandStormVisibilityMinRadius) * sandStormIntensity;
    }
}

// Ensures the internal render texture matches current window size
void GameMaster::ensureSandStormTexture(const sf::Vector2u &windowSize)
{
    if (sandStormMaskSize == windowSize)
        return;

    sandStormMaskSize = windowSize;
    sandStormRenderTexture.create(windowSize.x, windowSize.y);
    sandStormRenderTexture.setSmooth(true);
    sandStormSprite.setTexture(sandStormRenderTexture.getTexture());
}

// Renders the fog overlay and warning text for the sand storm
void GameMaster::renderSandStorm(sf::RenderWindow &window, const sf::Vector2f &focusPoint)
{
    const sf::Vector2u windowSize = window.getSize();

    if (sandStormWarningActive)
    {
        sf::Text warningText;
        warningText.setFont(ResourceManager::getInstance().getFont());
        warningText.setString("Sand storm approaching in about 3 seconds.");
        warningText.setCharacterSize(26);
        warningText.setFillColor(sf::Color(255, 235, 180));
        warningText.setOutlineColor(sf::Color(60, 40, 10, 200));
        warningText.setOutlineThickness(2.5f);
        sf::FloatRect bounds = warningText.getLocalBounds();
        warningText.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
        warningText.setPosition(windowSize.x / 2.f, 90.f);
        window.draw(warningText);
    }

    if (!sandStormInProgress)
    {
        return;
    }

    ensureSandStormTexture(windowSize);

    sandStormRenderTexture.clear(sf::Color(0, 0, 0, 0));

    sf::RectangleShape fogRect(sf::Vector2f(static_cast<float>(windowSize.x),
                                            static_cast<float>(windowSize.y)));
    fogRect.setFillColor(sf::Color(120, 85, 40, static_cast<sf::Uint8>(sandStormIntensity * sandStormFogMaxAlpha)));
    sandStormRenderTexture.draw(fogRect);

    // Create a uniform visibility circle around the player
    sf::CircleShape visibilityCircle(sandStormVisibilityRadius);
    visibilityCircle.setOrigin(sandStormVisibilityRadius, sandStormVisibilityRadius);
    visibilityCircle.setPosition(focusPoint);
    visibilityCircle.setFillColor(sf::Color(0, 0, 0, 0));
    sf::RenderStates clearState;
    clearState.blendMode = sf::BlendNone;
    sandStormRenderTexture.draw(visibilityCircle, clearState);

    sandStormRenderTexture.display();

    sandStormSprite.setPosition(0.f, 0.f);
    sandStormSprite.setColor(sf::Color::White);
    window.draw(sandStormSprite);
}

int GameMaster::getDefeatedEnemyCount(const GameWorld &gameWorld) const
{
    int defeated = 0;
    const auto &enemies = gameWorld.getEnemies();
    std::size_t enemyCount = gameWorld.getEnemyCount();

    for (std::size_t i = 0; i < enemyCount; ++i)
    {
        if (enemies[i] && !enemies[i]->isAlive())
        {
            ++defeated;
        }
    }
    return defeated;
}
