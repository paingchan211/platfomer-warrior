#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <string>

#include "Boss.h"
#include "Constants.h"
#include "DoublyLinkedList.h"
#include "Enemy.h"
#include "FireProjectile.h"
#include "IceProjectile.h"
#include "GameState.h"
#include "HPPotion.h"
#include "KeyBindingManager.h"
#include "Meteor.h"
#include "Platform.h"
#include "Player.h"
#include "ResourceManager.h"
#include "SinglyLinkedList.h"
#include "Stack.h"

class SaveGameManager;

// UISystem renders both in-game HUD components and menu overlays.
class UISystem
{
public:
    UISystem(ResourceManager &resourceManager);
    ~UISystem();

    void renderGameOverScreen(sf::RenderWindow &window,
                              bool gameWon,
                              const Player &player,
                              const std::unique_ptr<std::unique_ptr<Enemy>[]> &enemies,
                              std::size_t enemyCount,
                              const DoublyLinkedList<std::unique_ptr<FireProjectile>> &fireProjectiles,
                              const DoublyLinkedList<std::unique_ptr<HPPotion>> &hpPotions,
                              const DoublyLinkedList<std::unique_ptr<Meteor>> &meteors,
                              const std::unique_ptr<sf::Sprite[]> &floorTiles,
                              std::size_t floorTileCount,
                              const std::unique_ptr<std::unique_ptr<Platform>[]> &platforms,
                              std::size_t platformCount);

    // World entity rendering helpers
    void renderPlayerEntity(sf::RenderWindow &window, const Player &player, bool rageModeActive);
    void renderEnemyEntity(sf::RenderWindow &window, const Enemy &enemy);
    void renderBossEntity(sf::RenderWindow &window, const Boss &boss, bool bossRageModeActive);
    void renderPlatformEntity(sf::RenderWindow &window, const Platform &platform);
    void renderHPPotionEntity(sf::RenderWindow &window, const HPPotion &potion);
    void renderFireProjectileEntity(sf::RenderWindow &window, const FireProjectile &projectile);
    void renderIceProjectileEntity(sf::RenderWindow &window, const IceProjectile &projectile);
    void renderMeteorEntity(sf::RenderWindow &window, const Meteor &meteor);

    void renderHUD(sf::RenderWindow &window,
                   const Player &player,
                   int projectileAmmo,
                   float projectileCooldown,
                   int specialAttackAmmo,
                   float specialAttackCooldown,
                   bool isFireProjectile,
                   bool rageMode = false,
                   bool meteorFuryMode = false,
                   bool bossRageMode = false,
                   bool bossSpawning = false,
                   bool lateEnemyScalingAdjusted = false);

    void renderEnemyHealthBar(sf::RenderWindow &window,
                              const Enemy &enemy,
                              const sf::Vector2f &frameSize);

    void renderEnemyStatusEffects(sf::RenderWindow &window,
                                  const Enemy &enemy);

    void renderBossHealthBar(sf::RenderWindow &window,
                             const Boss &boss,
                             const sf::Vector2f &frameSize,
                             float scale);

    void renderBossStatusEffects(sf::RenderWindow &window,
                                 const Boss &boss,
                                 bool bossRageMode);

    void renderCenteredAlert(sf::RenderWindow &window,
                             const std::string &message,
                             float yOffset,
                             const sf::Color &textColor,
                             const sf::Color &boxColor,
                             unsigned int fontSize = 28);

    void renderCurrentMenus(sf::RenderWindow &window,
                            sf::View &camera,
                            const Stack<GameStateData> &stateStack,
                            Player *player,
                            Boss *boss,
                            SinglyLinkedList<std::string> &combatLog,
                            int combatLogCurrentNode,
                            int combatLogTraversalCount,
                            int combatLogDeleteCount,
                            const KeyBindingManager *keyManager = nullptr);

    void renderMainMenu(sf::RenderWindow &window, const GameStateData &mainMenuState);
    void renderPauseMenu(sf::RenderWindow &window, const GameStateData &pauseState);
    void renderLowHpWarning(sf::RenderWindow &window, float lowHpWarningTimer, const Player *player);
    void renderInventoryScreen(sf::RenderWindow &window, const Player *player);
    void renderPlayerStatsScreen(sf::RenderWindow &window, const Player *player);
    void renderInventoryOnlyScreen(sf::RenderWindow &window, const Player *player);
    void renderSkillTreeScreen(sf::RenderWindow &window, const Player *player, int selectedSkillIndex = 1);
    void renderHelpScreen(sf::RenderWindow &window);
    void renderCombatLog(sf::RenderWindow &window,
                         SinglyLinkedList<std::string> &combatLog,
                         int combatLogCurrentNode,
                         int combatLogTraversalCount,
                         int combatLogDeleteCount);
    void renderSettingsMenu(sf::RenderWindow &window, const GameStateData &settingsState);
    void renderAudioSettings(sf::RenderWindow &window, const GameStateData &audioState);
    void renderControlsMenu(sf::RenderWindow &window,
                            const GameStateData &controlsState,
                            const KeyBindingManager &keyManager);
    void renderConfirmDialog(sf::RenderWindow &window, const GameStateData &confirmState);
    void renderSaveGameMenu(sf::RenderWindow &window,
                            const GameStateData &saveState,
                            SaveGameManager *saveManager = nullptr);
    void renderLoadGameMenu(sf::RenderWindow &window,
                            const GameStateData &loadState,
                            SaveGameManager *saveManager = nullptr);
    void renderDebugMenu(sf::RenderWindow &window,
                         const GameStateData &debugState,
                         bool showStateStack,
                         bool showKeyDisplay,
                         bool showCollisions);
    void renderConsoleDebugMenu(sf::RenderWindow &window,
                                const GameStateData &debugState);

    void showToast(const std::string &message, sf::Color color = sf::Color::White);
    void updateToast(float dt);
    void renderToast(sf::RenderWindow &window);

    static sf::RectangleShape makeBar(sf::Vector2f position,
                                      sf::Vector2f size,
                                      const sf::Color &fill,
                                      const sf::Color &outline,
                                      float outlineThickness);

    static sf::Sprite makeTintedSprite(const sf::Sprite &sprite, const sf::Color &tint);

private:
    ResourceManager &resourceManager;

    void renderHealthBar(sf::RenderWindow &window, const Player &player);
    void renderXPBar(sf::RenderWindow &window, const Player &player);
    void renderInventoryDisplay(sf::RenderWindow &window, const Player &player);
    void renderProjectileAmmo(sf::RenderWindow &window, int ammo, float cooldown, const Player &player, bool isFireProjectile);
    void renderSpecialAttackAmmo(sf::RenderWindow &window, int ammo, float cooldown, bool unlocked);
    void renderControlsText(sf::RenderWindow &window);

    std::string toastMessage;
    float toastTimer;
    sf::Color toastColor;

    static constexpr float TOAST_DURATION = 2.0f;
};
