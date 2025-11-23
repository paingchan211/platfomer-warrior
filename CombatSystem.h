#pragma once
#include <functional>
#include <memory>
#include <random>
#include <string>

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Vector2.hpp>

#include "Constants.h"
#include "DoublyLinkedList.h"
#include "FireProjectile.h"
#include "IceProjectile.h"
#include "HPPotion.h"
#include "Meteor.h"
#include "ResourceManager.h"
#include "InputManager.h"
#include "GameState.h"
#include "KeyBindingManager.h"

class Player;
class Boss;
class Enemy;
class GameMaster;

// Combat system class that handles all combat-related mechanics including attacks, projectiles, and items
class CombatSystem
{
public:
    using CombatLogCallback = std::function<void(const std::string &)>;                           // Callback for combat log messages
    using FloatingTextCallback = std::function<void(int, sf::Vector2f, const sf::Color &, bool)>; // Callback for floating damage/heal text
    using PushStateCallback = std::function<void(GameStateType)>;                                 // Callback for pushing game states

    // Constructor that initializes the combat system with required managers and callbacks
    CombatSystem(ResourceManager &resourceManager,
                 InputManager &inputManager,
                 CombatLogCallback combatLogCallback,
                 FloatingTextCallback floatingTextCallback,
                 PushStateCallback pushStateCallback);

    // Resets the combat system state for a new game session
    void reset(std::mt19937 &rng, const Player &player);
    // Resets special attack hit tracking for enemies and boss
    void resetSpecialAttackHitTracking();

    // Updates player combat state including potion usage and cooldowns
    void updatePlayerCombat(float dt, Player &player, float slowMoPitchScale, const KeyBindingManager *keyManager = nullptr);

    // Handles player melee and special attacks against enemies and boss
    void handlePlayerAttacks(Player &player,
                             std::unique_ptr<std::unique_ptr<Enemy>[]> &enemies,
                             std::size_t enemiesCount,
                             Boss *boss,
                             bool bossSpawned,
                             bool &bossDeathSlowMotionActive,
                             float &bossDeathSlowMotionTimer,
                             float slowMotionDuration,
                             const KeyBindingManager *keyManager = nullptr,
                             float damageMultiplier = 1.0f);

    // Updates projectile system including firing, collisions, and ammo regeneration
    void updateProjectiles(float dt,
                           Player &player,
                           std::unique_ptr<std::unique_ptr<Enemy>[]> &enemies,
                           std::size_t enemiesCount,
                           Boss *boss,
                           bool bossSpawned,
                           bool &bossDeathSlowMotionActive,
                           float &bossDeathSlowMotionTimer,
                           float slowMotionDuration,
                           const KeyBindingManager *keyManager = nullptr);

    // Updates HP potion collection and usage
    void updatePotions(float dt, Player &player);
    // Updates meteor system including spawning and collisions
    void updateMeteors(float dt,
                       Player &player,
                       float groundLevel,
                       const sf::View &camera,
                       std::mt19937 &rng,
                       const GameMaster *gameMaster = nullptr,
                       int defeatedEnemiesCount = 0);

    // Returns current projectile ammo count
    int getProjectileAmmo() const;
    // Returns current projectile cooldown time
    float getProjectileCooldown() const;
    // Returns maximum projectile ammo based on player's skill tree
    int getMaxProjectileAmmo(const Player &player) const;
    // Returns current special attack ammo count
    int getSpecialAttackAmmo() const;
    // Returns current special attack cooldown time
    float getSpecialAttackCooldown() const;
    // Returns the effective projectile type based on player's unlocked skills
    bool isEffectivelyUsingFireProjectile(const Player &player) const;

    // Returns reference to active fire projectiles list
    DoublyLinkedList<std::unique_ptr<FireProjectile>> &getFireProjectiles();
    // Returns reference to active ice projectiles list
    DoublyLinkedList<std::unique_ptr<IceProjectile>> &getIceProjectiles();
    // Returns reference to active HP potions list
    DoublyLinkedList<std::unique_ptr<HPPotion>> &getHPPotions();
    // Returns reference to active meteors list
    DoublyLinkedList<std::unique_ptr<Meteor>> &getMeteors();

private:
    // Enumeration for projectile types
    enum class ProjectileType
    {
        FIRE, // Fire projectile type
        ICE   // Ice projectile type
    };

    // Handles collisions between projectiles and enemies/boss
    void handleProjectileCollisions(float dt,
                                    Player &player,
                                    std::unique_ptr<std::unique_ptr<Enemy>[]> &enemies,
                                    std::size_t enemiesCount,
                                    Boss *boss,
                                    bool bossSpawned,
                                    bool &bossDeathSlowMotionActive,
                                    float &bossDeathSlowMotionTimer,
                                    float slowMotionDuration);

    // Triggers boss defeat sequence with slow motion effect
    void triggerBossDefeated(bool &bossDeathSlowMotionActive,
                             float &bossDeathSlowMotionTimer,
                             float slowMotionDuration,
                             const char *debugMessage,
                             const std::string &logMessage);

    // Spawns or reuses a health potion at a given world position
    void spawnPotion(DoublyLinkedList<std::unique_ptr<HPPotion>> &potions,
                     std::size_t maxPotions,
                     const sf::Texture &texture,
                     sf::Vector2f worldPos);

    // Spawns or reuses a burst of meteors targeting random or player positions
    void spawnMeteorBurst(DoublyLinkedList<std::unique_ptr<Meteor>> &meteors,
                          std::size_t maxMeteors,
                          ResourceManager &resourceManager,
                          const sf::FloatRect &spawnBounds,
                          std::mt19937 &rng,
                          int count,
                          sf::Vector2f playerPos = sf::Vector2f(0.f, 0.f),
                          float playerTargetRatio = 0.0f);

    ResourceManager &resourceManager;          // Reference to resource manager for loading assets
    InputManager &inputManager;                // Reference to input manager for input handling
    CombatLogCallback combatLogCallback;       // Callback function for combat log messages
    FloatingTextCallback floatingTextCallback; // Callback function for floating text display
    PushStateCallback pushStateCallback;       // Callback function for pushing game states

    DoublyLinkedList<std::unique_ptr<FireProjectile>> fireProjectiles;    // Active fire projectiles
    DoublyLinkedList<std::unique_ptr<FireProjectile>> fireProjectilePool; // Pool of inactive fire projectiles for reuse
    DoublyLinkedList<std::unique_ptr<IceProjectile>> iceProjectiles;      // Active ice projectiles
    DoublyLinkedList<std::unique_ptr<IceProjectile>> iceProjectilePool;   // Pool of inactive ice projectiles for reuse
    DoublyLinkedList<std::unique_ptr<HPPotion>> hpPotions;                // Active HP potions in the world
    DoublyLinkedList<std::unique_ptr<HPPotion>> hpPotionPool;             // Pool of inactive HP potions for reuse
    DoublyLinkedList<std::unique_ptr<Meteor>> meteors;                    // Active meteors in the world
    DoublyLinkedList<std::unique_ptr<Meteor>> meteorPool;                 // Pool of inactive meteors for reuse

    std::unique_ptr<bool[]> enemySpecialAttackHit; // Array tracking which enemies have been hit by special attack
    std::size_t enemySpecialAttackCount;           // Number of enemies in the tracking array
    bool bossSpecialAttackHit;                     // Flag tracking if boss has been hit by special attack

    int projectileAmmo;                   // Current projectile ammo count
    float projectileCooldown;             // Cooldown timer for projectile regeneration
    ProjectileType currentProjectileType; // Currently selected projectile type
    int specialAttackAmmo;                // Current special attack ammo count
    float specialAttackCooldown;          // Cooldown timer for special attack regeneration
    float potionUseCooldown;              // Cooldown timer for potion usage
    float attackSoundDelay;               // Delay timer for attack sound playback
    bool attackSoundPending;              // Flag indicating if attack sound is pending
    int pendingAttackType;                // Type of attack sound that is pending

    float meteorShowerTimer; // Timer tracking time until next meteor shower
    float nextMeteorShower;  // Time interval until next meteor shower spawns
};
