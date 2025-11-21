#include "CombatSystem.h"

#include <algorithm>
#include <cstddef>
#include <cmath>
#include <iostream>
#include <utility>

#include <SFML/Window/Keyboard.hpp>

#include "Boss.h"
#include "Enemy.h"
#include "Player.h"
#include "GameMaster.h"
#include "PolymorphismLogger.h"

// Anonymous namespace for internal helper utilities not exposed outside this file
namespace
{
    template <typename T>
    void logListState(const char *label,
                      const DoublyLinkedList<std::unique_ptr<T>> &active,
                      const DoublyLinkedList<std::unique_ptr<T>> &pool)
    {
        if (!ENABLE_DOUBLY_LINKED_LIST_STDOUT)
        {
            return;
        }

        if (!label || label[0] == '\0')
            label = "List";

        std::cout << "[DoublyLinkedList] " << label << " | active: " << active.size()
                  << " | pool: " << pool.size() << " ";
    }

    // Helper function template to reclaim inactive items from active list to pool for reuse
    template <typename T>
    void reclaimInactive(DoublyLinkedList<std::unique_ptr<T>> &active,
                         DoublyLinkedList<std::unique_ptr<T>> &pool,
                         const char *label)
    {
        std::size_t reclaimed = 0;
        // Iterate through active items and move inactive ones to pool
        for (auto it = active.begin(); it != active.end();)
        {
            auto &item = *it;
            // Check if item is null or inactive
            if (!item || !item->isActive())
            {
                // Move inactive item to pool for reuse
                pool.pushBack(std::move(item));
                it = active.erase(it); // Erase from active list and advance iterator
                ++reclaimed;
            }
            else
            {
                ++it; // Skip active element
            }
        }

        if (reclaimed > 0 && ENABLE_DOUBLY_LINKED_LIST_STDOUT)
        {
            logListState(label, active, pool);
            std::cout << "    action: reclaimed " << reclaimed << " inactive node(s) into pool" << std::endl;

            if (ENABLE_ITERATOR_STDOUT)
                std::cout << "[Iterator] Traditional iterator (begin/end/erase) used to traverse and remove "
                          << reclaimed << " inactive " << label << " node(s)" << std::endl;
        }
    }
}

// Constructor that initializes the combat system with required managers and callbacks
CombatSystem::CombatSystem(ResourceManager &resourceManager,
                           InputManager &inputManager,
                           CombatLogCallback combatLogCallback,
                           FloatingTextCallback floatingTextCallback,
                           PushStateCallback pushStateCallback)
    : resourceManager(resourceManager),
      inputManager(inputManager),
      combatLogCallback(std::move(combatLogCallback)),
      floatingTextCallback(std::move(floatingTextCallback)),
      pushStateCallback(std::move(pushStateCallback)),
      enemySpecialAttackHit(nullptr),
      enemySpecialAttackCount(0),
      bossSpecialAttackHit(false),
      projectileAmmo(MAX_PROJECTILE_AMMO),
      projectileCooldown(0.f),
      currentProjectileType(ProjectileType::FIRE),
      specialAttackAmmo(MAX_SPECIAL_ATTACK_AMMO),
      specialAttackCooldown(0.f),
      potionUseCooldown(0.f),
      attackSoundDelay(0.f),
      attackSoundPending(false),
      pendingAttackType(0),
      meteorShowerTimer(0.f),
      nextMeteorShower(0.f)
{
    // Constructor body is empty; all initialization is done in the member initializer list
}

// Resets the combat system state for a new game session
void CombatSystem::reset(std::mt19937 &rng, const Player &player)
{
    // Clear all active and pooled projectiles, potions, and meteors
    fireProjectiles.clear();
    fireProjectilePool.clear();
    iceProjectiles.clear();
    iceProjectilePool.clear();
    hpPotions.clear();
    hpPotionPool.clear();
    meteors.clear();
    meteorPool.clear();

    // Reset projectile ammo and cooldown based on player's skill tree
    projectileAmmo = getMaxProjectileAmmo(player);
    projectileCooldown = 0.f;

    // Determine projectile type based on player's unlocked skills
    bool hasFire = player.getSkillTree().hasFireProjectile();
    bool hasIce = player.getSkillTree().hasIceProjectile();
    if (hasFire)
        currentProjectileType = ProjectileType::FIRE;
    else if (hasIce)
        currentProjectileType = ProjectileType::ICE;
    else
        currentProjectileType = ProjectileType::FIRE;

    // Reset special attack and potion cooldowns (respect unlocked skills)
    const bool hasSpecialAttack = player.getSkillTree().hasSpecialAttack();
    specialAttackAmmo = hasSpecialAttack ? MAX_SPECIAL_ATTACK_AMMO : 0;
    specialAttackCooldown = 0.f;
    potionUseCooldown = 0.f;
    attackSoundDelay = 0.f;
    attackSoundPending = false;
    pendingAttackType = 0;

    // Reset meteor shower timer with random interval
    meteorShowerTimer = 0.f;
    std::uniform_real_distribution<float> meteorInterval(10.f, 20.f);
    nextMeteorShower = meteorInterval(rng);

    // Reset special attack hit tracking
    resetSpecialAttackHitTracking();
    bossSpecialAttackHit = false;
}

// Resets special attack hit tracking for enemies and boss
void CombatSystem::resetSpecialAttackHitTracking()
{
    // Reset enemy special attack hit flags if array exists
    if (enemySpecialAttackHit)
    {
        for (std::size_t i = 0; i < enemySpecialAttackCount; ++i)
        {
            enemySpecialAttackHit[i] = false; // Mark each enemy as not hit by the special attack
        }
    }
    else
    {
        enemySpecialAttackCount = 0; // No enemies tracked yet
    }
    // Reset boss special attack hit flag
    bossSpecialAttackHit = false;
}

// Updates player combat state including potion usage and cooldowns
void CombatSystem::updatePlayerCombat(float dt, Player &player, float slowMoPitchScale, const KeyBindingManager *keyManager)
{
    // Handle delayed attack sound playback
    if (attackSoundPending)
    {
        attackSoundDelay -= dt;
        // Play attack sound when delay expires
        if (attackSoundDelay <= 0.f)
        {
            const float soundPitch = slowMoPitchScale;
            resourceManager.playAttackSound(pendingAttackType, soundPitch);
            attackSoundPending = false;
            pendingAttackType = 0;
        }
    }

    // Get potion key from key manager or use default
    sf::Keyboard::Key potionKey = sf::Keyboard::Q;
    if (keyManager)
    {
        potionKey = keyManager->getKeyForAction(GameAction::USE_POTION);
    }

    // Handle potion usage if key is pressed and cooldown is ready
    if (sf::Keyboard::isKeyPressed(potionKey) && potionUseCooldown <= 0.f)
    {
        int hpBefore = player.getHp();
        // Try to use HP potion from inventory
        if (player.useHpPotion())
        {
            potionUseCooldown = 0.5f; // Small delay before potion can be used again

            // Calculate heal amount and display floating text
            int healAmount = player.getHp() - hpBefore;
            if (healAmount > 0)
            {
                sf::FloatRect playerBounds = player.getCollisionBounds();
                // Display floating green text for healing
                if (floatingTextCallback)
                {
                    floatingTextCallback(healAmount,
                                         {playerBounds.left + playerBounds.width / 2.f, playerBounds.top},
                                         sf::Color::Green,
                                         true);
                }
                // Log potion usage to combat log
                if (combatLogCallback)
                {
                    combatLogCallback("Used HP Potion! Restored " + std::to_string(healAmount) + " HP");
                }
            }
        }
    }

    // Update potion use cooldown
    if (potionUseCooldown > 0.f)
    {
        potionUseCooldown -= dt;
    }

    const bool hasSpecialAttack = player.getSkillTree().hasSpecialAttack();
    if (!hasSpecialAttack)
    {
        specialAttackAmmo = 0;
        specialAttackCooldown = 0.f;
    }
    else
    {
        // Update special attack cooldown
        if (specialAttackCooldown > 0.f)
        {
            specialAttackCooldown -= dt;
            if (specialAttackCooldown < 0.f)
                specialAttackCooldown = 0.f;
        }

        // Regenerate special attack ammo if below maximum and cooldown is ready
        if (specialAttackAmmo < MAX_SPECIAL_ATTACK_AMMO && specialAttackCooldown <= 0.f)
        {
            ++specialAttackAmmo;
            // Set cooldown if ammo is still below maximum
            if (specialAttackAmmo < MAX_SPECIAL_ATTACK_AMMO)
            {
                specialAttackCooldown = SPECIAL_ATTACK_COOLDOWN_TIME;
            }
        }
    }
}

// Handles player melee and special attacks against enemies and boss
void CombatSystem::handlePlayerAttacks(Player &player,
                                       std::unique_ptr<std::unique_ptr<Enemy>[]> &enemies,
                                       std::size_t enemiesCount,
                                       Boss *boss,
                                       bool bossSpawned,
                                       bool &bossDeathSlowMotionActive,
                                       float &bossDeathSlowMotionTimer,
                                       float slowMotionDuration,
                                       const KeyBindingManager *keyManager,
                                       float damageMultiplier)
{
    // Get player collision bounds and stats
    const sf::FloatRect playerCollision = player.getCollisionBounds();

    // Lambda function to process melee attack against enemies and boss
    auto processMeleeAttack = [&](int attackType)
    {
        // Trigger player attack animation and sound
        player.attack(attackType);
        attackSoundPending = true;
        attackSoundDelay = 0.15f; // Delay to sync sound with animation
        pendingAttackType = attackType;
        const sf::FloatRect attackRange = player.getAttackRange();

        // Check collision with each enemy
        for (std::size_t ei = 0; ei < enemiesCount; ++ei)
        {
            auto &enemy = enemies[ei];
            if (!enemy || !enemy->isAlive())
                continue;

            // If enemy is inside melee range
            if (checkRectIntersection(attackRange, enemy->getCollisionBounds()))
            {
                int damage = static_cast<int>(player.getDamage() * damageMultiplier);
                enemy->takeDamage(damage);
                auto levelUpInfo = player.gainExperience(10);
                player.applyLevelUpBonuses(levelUpInfo);

                sf::FloatRect enemyBounds = enemy->getCollisionBounds();
                if (floatingTextCallback)
                {
                    floatingTextCallback(damage,
                                         {enemyBounds.left + enemyBounds.width / 2.f, enemyBounds.top},
                                         sf::Color::Yellow,
                                         false);
                }

                if (combatLogCallback)
                {
                    combatLogCallback("Dealt " + std::to_string(damage) + " damage to Enemy");
                }

                // Handle enemy death from melee attack
                if (!enemy->isAlive())
                {
                    const sf::FloatRect enemyDeathBounds = enemy->getCollisionBounds();
                    const sf::Vector2f potionPos{enemyDeathBounds.left + enemyDeathBounds.width / 2.f - 18.f,
                                                 enemyDeathBounds.top + enemyDeathBounds.height - 56.f};
                    // Spawn HP potion at enemy death location
                    spawnPotion(hpPotions, MAX_HP_POTIONS, resourceManager.getTexture("hp_potion"), potionPos);
                    auto levelUpInfo = player.gainExperience(25);
                    player.applyLevelUpBonuses(levelUpInfo);

                    // Reward skill point on enemy kill
                    player.getSkillTree().addSkillPoints(1);

                    // Deactivate the enemy so it's no longer rendered or updated
                    enemy->setActive(false);

                    if (combatLogCallback)
                    {
                        combatLogCallback("Defeated Enemy! (+25 XP, +1 Skill Point)");
                    }
                }
            }
        }

        // Handle boss hit by melee
        if (bossSpawned && boss && boss->isAlive() && checkRectIntersection(attackRange, boss->getCollisionBounds()))
        {
            int damage = static_cast<int>(player.getDamage() * damageMultiplier);
            boss->takeDamage(damage);
            auto levelUpInfo = player.gainExperience(20);
            player.applyLevelUpBonuses(levelUpInfo);

            sf::FloatRect bossBounds = boss->getCollisionBounds();
            if (floatingTextCallback)
            {
                floatingTextCallback(damage,
                                     {bossBounds.left + bossBounds.width / 2.f, bossBounds.top},
                                     sf::Color::Yellow,
                                     false);
            }

            if (combatLogCallback)
            {
                combatLogCallback("Dealt " + std::to_string(damage) + " damage to BOSS!");
            }

            // If boss dies from melee, trigger slow motion victory sequence
            if (!boss->isAlive())
            {
                auto levelUpInfo = player.gainExperience(100);
                player.applyLevelUpBonuses(levelUpInfo);
                triggerBossDefeated(bossDeathSlowMotionActive,
                                    bossDeathSlowMotionTimer,
                                    slowMotionDuration,
                                    "Boss defeated by player melee! Starting slow motion...",
                                    "*** BOSS DEFEATED! Victory! (+100 XP) ***");
            }
        }
    };

    // Resolve key bindings for melee and special attack
    const bool attack1Down = sf::Keyboard::isKeyPressed(keyManager ? keyManager->getKeyForAction(GameAction::MELEE_ATTACK) : sf::Keyboard::A);
    // Only trigger melee attack on key press edge (not while holding)
    if (attack1Down && !inputManager.isAttack1KeyPressed() && !player.getIsAttacking() && !player.getIsSpecialAttacking())
    {
        processMeleeAttack(1);
    }
    inputManager.setAttack1KeyPressed(attack1Down);

    // Handle special attack input
    const bool specialKeyDown = sf::Keyboard::isKeyPressed(keyManager ? keyManager->getKeyForAction(GameAction::SPECIAL_ATTACK) : sf::Keyboard::S);
    const bool hasSpecialAttack = player.getSkillTree().hasSpecialAttack();
    if (specialKeyDown && hasSpecialAttack && !inputManager.isAttack2KeyPressed() && !player.getIsAttacking() && !player.getIsSpecialAttacking() && specialAttackAmmo > 0)
    {
        // Start special attack animation
        player.specialAttack();
        attackSoundPending = true;
        attackSoundDelay = 0.05f; // Shorter delay for special attack sound
        pendingAttackType = 3;

        // Reset tracking so special attack can hit each enemy once
        if (enemySpecialAttackHit)
        {
            for (std::size_t i = 0; i < enemySpecialAttackCount; ++i)
            {
                enemySpecialAttackHit[i] = false;
            }
        }
        bossSpecialAttackHit = false;

        // Consume one special attack ammo and refresh cooldown if needed
        --specialAttackAmmo;
        if (specialAttackAmmo < MAX_SPECIAL_ATTACK_AMMO)
        {
            specialAttackCooldown = SPECIAL_ATTACK_COOLDOWN_TIME;
        }
    }
    inputManager.setAttack2KeyPressed(specialKeyDown);

    // While special attack animation is active, apply AoE damage to enemies and boss
    if (player.getIsSpecialAttacking())
    {
        const sf::FloatRect attackRange = player.getAttackRange();

        // Apply special attack to enemies
        for (std::size_t i = 0; i < enemiesCount; ++i)
        {
            auto &enemy = enemies[i];
            if (!enemy || !enemy->isAlive())
                continue;
            if (enemySpecialAttackHit && enemySpecialAttackHit[i])
                continue; // Skip enemies already hit by this special

            if (checkRectIntersection(attackRange, enemy->getCollisionBounds()))
            {
                int damage = static_cast<int>(player.getDamage() * 2 * damageMultiplier);
                enemy->takeDamage(damage);
                auto levelUpInfo = player.gainExperience(15);
                player.applyLevelUpBonuses(levelUpInfo);
                if (enemySpecialAttackHit && i < enemySpecialAttackCount)
                {
                    enemySpecialAttackHit[i] = true; // Mark enemy as hit
                }

                sf::FloatRect enemyBounds = enemy->getCollisionBounds();
                if (floatingTextCallback)
                {
                    floatingTextCallback(damage,
                                         {enemyBounds.left + enemyBounds.width / 2.f, enemyBounds.top},
                                         sf::Color::Cyan,
                                         false);
                }

                if (combatLogCallback)
                {
                    combatLogCallback("Special Attack! Dealt " + std::to_string(damage) + " damage to Enemy");
                }

                // Handle enemy death due to special attack
                if (!enemy->isAlive())
                {
                    const sf::FloatRect enemyDeathBounds = enemy->getCollisionBounds();
                    const sf::Vector2f potionPos{enemyDeathBounds.left + enemyDeathBounds.width / 2.f - 18.f,
                                                 enemyDeathBounds.top + enemyDeathBounds.height - 56.f};
                    spawnPotion(hpPotions, MAX_HP_POTIONS, resourceManager.getTexture("hp_potion"), potionPos);

                    player.getSkillTree().addSkillPoints(1);

                    // Deactivate the enemy so it's no longer rendered or updated
                    enemy->setActive(false);

                    if (combatLogCallback)
                    {
                        combatLogCallback("Defeated Enemy with Special Attack! (+25 XP, +1 Skill Point)");
                    }
                }
            }
        }

        // Apply special attack to boss
        if (boss && boss->isAlive() && !bossSpecialAttackHit)
        {
            if (checkRectIntersection(attackRange, boss->getCollisionBounds()))
            {
                int damage = static_cast<int>(player.getDamage() * 2 * damageMultiplier);
                boss->takeDamage(damage);
                auto levelUpInfo = player.gainExperience(20);
                player.applyLevelUpBonuses(levelUpInfo);
                bossSpecialAttackHit = true; // Ensure boss only gets hit once by this special

                sf::FloatRect bossBounds = boss->getCollisionBounds();
                if (floatingTextCallback)
                {
                    floatingTextCallback(damage,
                                         {bossBounds.left + bossBounds.width / 2.f, bossBounds.top},
                                         sf::Color::Cyan,
                                         false);
                }

                if (combatLogCallback)
                {
                    combatLogCallback("Special Attack! Dealt " + std::to_string(damage) + " damage to BOSS!");
                }

                // Trigger slow motion when boss dies to special attack
                if (!boss->isAlive())
                {
                    auto levelUpInfo = player.gainExperience(100);
                    player.applyLevelUpBonuses(levelUpInfo);
                    triggerBossDefeated(bossDeathSlowMotionActive,
                                        bossDeathSlowMotionTimer,
                                        slowMotionDuration,
                                        "Boss defeated by special attack! Starting slow motion...",
                                        "*** BOSS DEFEATED WITH SPECIAL ATTACK! Victory! (+100 XP) ***");
                }
            }
        }
    }
}

// Updates projectile firing, switching, and regeneration logic
void CombatSystem::updateProjectiles(float dt,
                                     Player &player,
                                     std::unique_ptr<std::unique_ptr<Enemy>[]> &enemies,
                                     std::size_t enemiesCount,
                                     Boss *boss,
                                     bool bossSpawned,
                                     bool &bossDeathSlowMotionActive,
                                     float &bossDeathSlowMotionTimer,
                                     float slowMotionDuration,
                                     const KeyBindingManager *keyManager)
{
    // Decrease projectile cooldown over time
    if (projectileCooldown > 0.f)
        projectileCooldown -= dt;

    // Default keys for switching and shooting projectiles
    sf::Keyboard::Key switchKey = sf::Keyboard::R;
    sf::Keyboard::Key shootKey = sf::Keyboard::F;
    if (keyManager)
    {
        switchKey = keyManager->getKeyForAction(GameAction::SWITCH_PROJECTILE);
        shootKey = keyManager->getKeyForAction(GameAction::SHOOT_PROJECTILE);
    }

    // Handle projectile type switch input
    const bool switchProjectileDown = sf::Keyboard::isKeyPressed(switchKey);
    if (switchProjectileDown && !inputManager.isSwitchProjectileKeyPressed())
    {
        // Check if both projectile types are unlocked
        bool hasFire = player.getSkillTree().hasFireProjectile();
        bool hasIce = player.getSkillTree().hasIceProjectile();

        if (hasFire && hasIce)
        {
            // Toggle between fire and ice projectile types
            currentProjectileType = (currentProjectileType == ProjectileType::FIRE) ? ProjectileType::ICE : ProjectileType::FIRE;

            // Adjust ammo to the new max if current ammo exceeds it
            int newMaxAmmo = getMaxProjectileAmmo(player);
            if (projectileAmmo > newMaxAmmo)
                projectileAmmo = newMaxAmmo;

            if (combatLogCallback)
            {
                std::string projectileName = (currentProjectileType == ProjectileType::FIRE) ? "Fire" : "Ice";
                combatLogCallback("Switched to " + projectileName + " Projectile");
            }
        }
    }
    inputManager.setSwitchProjectileKeyPressed(switchProjectileDown);

    // Handle projectile firing input
    const bool projectileKeyDown = sf::Keyboard::isKeyPressed(shootKey);
    if (projectileKeyDown && !inputManager.isProjectileKeyPressed() && projectileAmmo > 0)
    {
        // Determine the direction the player is facing
        const bool playerFacingRight = player.isFacingRight();

        // Resolve effective projectile type in case selected type is locked
        ProjectileType effectiveType = currentProjectileType;
        if (effectiveType == ProjectileType::FIRE && !player.getSkillTree().hasFireProjectile() && player.getSkillTree().hasIceProjectile())
            effectiveType = ProjectileType::ICE;
        else if (effectiveType == ProjectileType::ICE && !player.getSkillTree().hasIceProjectile() && player.getSkillTree().hasFireProjectile())
            effectiveType = ProjectileType::FIRE;

        int spawnedFireProjectiles = 0;
        int spawnedIceProjectiles = 0;

        // Handle fire projectile logic
        if (effectiveType == ProjectileType::FIRE)
        {
            // Prevent firing fire projectile if not unlocked
            if (!player.getSkillTree().hasFireProjectile())
            {
                if (combatLogCallback)
                    combatLogCallback("Fire Projectile locked! Unlock in Skill Tree (T key)");
                inputManager.setProjectileKeyPressed(projectileKeyDown);
                return;
            }

            int fireCount = player.getSkillTree().getFireProjectileCount();

            // Spawn multiple fire projectiles based on player upgrades
            for (int i = 0; i < fireCount; ++i)
            {
                float offsetX = playerFacingRight ? 50.f : -50.f;
                float offsetY = -130.f + (i * 20.f); // Vertical spread between projectiles

                const sf::Vector2f spawnPos{
                    player.getPosition().x + offsetX,
                    player.getPosition().y + player.getCollisionOffsetY() + player.getCollisionHeight() / 10.f + offsetY};

                std::unique_ptr<FireProjectile> newProj(nullptr);
                // Reuse projectile from pool if available
                if (!fireProjectilePool.isEmpty())
                {
                    newProj = std::move(fireProjectilePool.front());
                    fireProjectilePool.popFront();

                    if (newProj)
                        newProj->reset(resourceManager.getTexture("fireball"), spawnPos, playerFacingRight);
                }
                // Otherwise, allocate a new projectile if we are under the limit
                else if (fireProjectiles.size() < MAX_PROJECTILES)
                {
                    newProj = std::make_unique<FireProjectile>(resourceManager.getTexture("fireball"), spawnPos, playerFacingRight);
                }

                // Activate projectile if successfully created
                if (newProj)
                {
                    fireProjectiles.pushBack(std::move(newProj));
                    resourceManager.playFireballSound();
                    ++spawnedFireProjectiles;
                }
            }
        }
        // Handle ice projectile logic
        else
        {
            // Prevent firing ice projectile if not unlocked
            if (!player.getSkillTree().hasIceProjectile())
            {
                if (combatLogCallback)
                    combatLogCallback("Ice Projectile locked! Unlock in Skill Tree (T key)");
                inputManager.setProjectileKeyPressed(projectileKeyDown);
                return;
            }

            int iceCount = player.getSkillTree().getIceProjectileCount();

            // Spawn multiple ice projectiles based on player upgrades
            for (int i = 0; i < iceCount; ++i)
            {
                float offsetX = playerFacingRight ? 50.f : -50.f;
                float offsetY = -40.f + (i * 20.f); // Different vertical spread for ice

                const sf::Vector2f spawnPos{
                    player.getPosition().x + offsetX,
                    player.getPosition().y + player.getCollisionOffsetY() + player.getCollisionHeight() / 2.f + offsetY};

                std::unique_ptr<IceProjectile> newProj(nullptr);
                // Reuse projectile from pool if available
                if (!iceProjectilePool.isEmpty())
                {
                    newProj = std::move(iceProjectilePool.front());
                    iceProjectilePool.popFront();

                    if (newProj)
                        newProj->reset(resourceManager.getTexture("ice_projectile"), spawnPos, playerFacingRight);
                }
                // Otherwise, allocate a new projectile if under limit
                else if (iceProjectiles.size() < MAX_PROJECTILES)
                {
                    newProj = std::make_unique<IceProjectile>(resourceManager.getTexture("ice_projectile"), spawnPos, playerFacingRight);
                }

                // Activate projectile if successfully created
                if (newProj)
                {
                    iceProjectiles.pushBack(std::move(newProj));
                    resourceManager.playFireballSound();
                    ++spawnedIceProjectiles;
                }
            }
        }

        if (ENABLE_DOUBLY_LINKED_LIST_STDOUT)
        {
            if (spawnedFireProjectiles > 0)
            {
                logListState("Fire Projectiles", fireProjectiles, fireProjectilePool);
                std::cout << "    action: spawned " << spawnedFireProjectiles << " fire projectile(s)" << std::endl;
            }
            else if (spawnedIceProjectiles > 0)
            {
                logListState("Ice Projectiles", iceProjectiles, iceProjectilePool);
                std::cout << "    action: spawned " << spawnedIceProjectiles << " ice projectile(s)" << std::endl;
            }
        }

        // Consume one ammo per "shot" (regardless of how many projectiles per shot)
        --projectileAmmo;

        // Start recharge cooldown if not at max ammo
        int maxAmmo = getMaxProjectileAmmo(player);
        if (projectileAmmo < maxAmmo)
            projectileCooldown = PROJECTILE_COOLDOWN_TIME;
    }
    inputManager.setProjectileKeyPressed(projectileKeyDown);

    // Passive ammo regeneration over time if cooldown has expired
    int maxAmmo = getMaxProjectileAmmo(player);
    if (projectileAmmo < maxAmmo && projectileCooldown <= 0.f)
    {
        ++projectileAmmo;
        if (projectileAmmo < maxAmmo)
        {
            projectileCooldown = PROJECTILE_COOLDOWN_TIME;
        }
    }

    // Handle collisions and effects produced by active projectiles
    handleProjectileCollisions(dt,
                               player,
                               enemies,
                               enemiesCount,
                               boss,
                               bossSpawned,
                               bossDeathSlowMotionActive,
                               bossDeathSlowMotionTimer,
                               slowMotionDuration);

    // Reclaim inactive fire projectiles to pool
    int fireReclaimed = 0;
    for (auto it = fireProjectiles.begin(); it != fireProjectiles.end();)
    {
        auto &projPtr = *it;
        if (!projPtr || !projPtr->isActive())
        {
            fireProjectilePool.pushBack(std::move(projPtr));
            it = fireProjectiles.erase(it);
            ++fireReclaimed;
        }
        else
            ++it;
    }

    if (fireReclaimed > 0 && ENABLE_DOUBLY_LINKED_LIST_STDOUT)
    {
        logListState("Fire Projectiles", fireProjectiles, fireProjectilePool);
        std::cout << "    action: reclaimed " << fireReclaimed << " fire projectile(s) into pool" << std::endl;
    }

    // Reclaim inactive ice projectiles to pool
    int iceReclaimed = 0;
    for (auto it = iceProjectiles.begin(); it != iceProjectiles.end();)
    {
        auto &projPtr = *it;
        if (!projPtr || !projPtr->isActive())
        {
            iceProjectilePool.pushBack(std::move(projPtr));
            it = iceProjectiles.erase(it);
            ++iceReclaimed;
        }
        else
            ++it;
    }

    if (iceReclaimed > 0 && ENABLE_DOUBLY_LINKED_LIST_STDOUT)
    {
        logListState("Ice Projectiles", iceProjectiles, iceProjectilePool);
        std::cout << "    action: reclaimed " << iceReclaimed << " ice projectile(s) into pool" << std::endl;
    }
}

// Handles projectile movement, collision detection, and damage application
void CombatSystem::handleProjectileCollisions(float dt,
                                              Player &player,
                                              std::unique_ptr<std::unique_ptr<Enemy>[]> &enemies,
                                              std::size_t enemiesCount,
                                              Boss *boss,
                                              bool bossSpawned,
                                              bool &bossDeathSlowMotionActive,
                                              float &bossDeathSlowMotionTimer,
                                              float slowMotionDuration)
{

    // Process all fire projectiles
    static int fireFrameCounter = 0;
    fireFrameCounter++;

    int fireIndex = 0;
    for (auto it = fireProjectiles.begin(); it != fireProjectiles.end(); ++it)
    {
        auto &projectile = *it;
        Projectile *baseProjectile = projectile.get();
        if (!baseProjectile || !baseProjectile->isActive())
            continue;

        // Update projectile position and state
        if (ENABLE_POLYMORPHISM_STDOUT && fireFrameCounter == 120 && fireIndex == 0)
        {
            std::cout << "  - FireProjectile[" << fireIndex << "]::update() called (animating)\n";
        }
        logPolymorphicDispatch(baseProjectile, "Projectile", "update(float)");
        baseProjectile->update(dt);
        fireIndex++;
        const sf::FloatRect projectileBounds = baseProjectile->getBounds();

        // Collision with enemies
        for (std::size_t ei = 0; ei < enemiesCount; ++ei)
        {
            auto &enemy = enemies[ei];
            if (!enemy || !enemy->isAlive())
                continue;

            if (checkRectIntersection(projectileBounds, enemy->getCollisionBounds()))
            {
                // Calculate damage with fire bonuses
                int baseDamage = projectile->getDamage();
                int damageBonus = player.getSkillTree().getTotalFireDamage();
                int damage = baseDamage + damageBonus;

                enemy->takeDamage(damage);

                // Apply burning effect over time
                enemy->applyBurn(projectile->getBurnDamage(), projectile->getBurnDuration());

                // Deactivate projectile after hit
                baseProjectile->setActive(false);
                auto levelUpInfo = player.gainExperience(10);
                player.applyLevelUpBonuses(levelUpInfo);

                sf::FloatRect enemyBounds = enemy->getCollisionBounds();
                if (floatingTextCallback)
                {
                    floatingTextCallback(damage,
                                         {enemyBounds.left + enemyBounds.width / 2.f, enemyBounds.top},
                                         sf::Color(255, 150, 50),
                                         false);
                }

                if (combatLogCallback)
                {
                    // Note: text mentions "Ice Projectile" but this is fire; likely a leftover message
                    combatLogCallback("Ice Projectile hit! Dealt " + std::to_string(damage) + " damage to Enemy [FROZEN/SLOWED]");
                }

                // Handle enemy death from fire projectile
                if (!enemy->isAlive())
                {
                    const sf::FloatRect enemyDeathBounds = enemy->getCollisionBounds();
                    spawnPotion(hpPotions, MAX_HP_POTIONS, resourceManager.getTexture("hp_potion"),
                                {enemyDeathBounds.left + enemyDeathBounds.width / 2.f - 18.f,
                                 enemyDeathBounds.top + enemyDeathBounds.height - 56.f});
                    auto levelUpInfo = player.gainExperience(25);
                    player.applyLevelUpBonuses(levelUpInfo);

                    player.getSkillTree().addSkillPoints(1);

                    // Deactivate the enemy so it's no longer rendered or updated
                    enemy->setActive(false);

                    if (combatLogCallback)
                    {
                        combatLogCallback("Defeated Enemy! (+25 XP, +1 Skill Point)");
                    }
                }
                break; // Only one enemy can be hit per projectile
            }
        }

        // Collision with boss (only if still active after enemy checks)
        if (projectile && projectile->isActive() && bossSpawned && boss && boss->isAlive() && checkRectIntersection(projectileBounds, boss->getCollisionBounds()))
        {
            int baseDamage = projectile->getDamage();
            int damageBonus = player.getSkillTree().getTotalFireDamage();
            int damage = baseDamage + damageBonus;

            boss->takeDamage(damage);

            // Apply burning DoT to boss
            boss->applyBurn(projectile->getBurnDamage(), projectile->getBurnDuration());

            projectile->setActive(false);
            auto levelUpInfo = player.gainExperience(20);
            player.applyLevelUpBonuses(levelUpInfo);

            sf::FloatRect bossBounds = boss->getCollisionBounds();
            if (floatingTextCallback)
            {
                floatingTextCallback(damage,
                                     {bossBounds.left + bossBounds.width / 2.f, bossBounds.top},
                                     sf::Color(255, 150, 50),
                                     false);
            }

            if (combatLogCallback)
            {
                combatLogCallback("Fire Projectile hit! Dealt " + std::to_string(damage) + " damage to BOSS! [BURNING]");
            }

            // If boss dies, trigger slow motion victory
            if (!boss->isAlive())
            {
                auto levelUpInfo = player.gainExperience(100);
                player.applyLevelUpBonuses(levelUpInfo);
                triggerBossDefeated(bossDeathSlowMotionActive,
                                    bossDeathSlowMotionTimer,
                                    slowMotionDuration,
                                    "Boss defeated by fire projectile! Starting slow motion...",
                                    "*** BOSS DEFEATED WITH FIRE PROJECTILE! Victory! (+100 XP) ***");
            }
        }
    }

    // Process all ice projectiles
    static int iceFrameCounter = 0;
    iceFrameCounter++;

    int iceIndex = 0;
    for (auto it = iceProjectiles.begin(); it != iceProjectiles.end(); ++it)
    {
        auto &projectile = *it;
        Projectile *baseProjectile = projectile.get();
        if (!baseProjectile || !baseProjectile->isActive())
            continue;

        // Update projectile position and state
        if (ENABLE_POLYMORPHISM_STDOUT && iceFrameCounter == 120 && iceIndex == 0)
        {
            std::cout << "  - IceProjectile[" << iceIndex << "]::update() called (animating)\n";
        }
        logPolymorphicDispatch(baseProjectile, "Projectile", "update(float)");
        baseProjectile->update(dt);
        iceIndex++;
        const sf::FloatRect projectileBounds = baseProjectile->getBounds();

        // Collision with enemies
        for (std::size_t ei = 0; ei < enemiesCount; ++ei)
        {
            auto &enemy = enemies[ei];
            if (!enemy || !enemy->isAlive())
                continue;
            if (checkRectIntersection(projectileBounds, enemy->getCollisionBounds()))
            {
                int baseDamage = projectile->getDamage();
                int damageBonus = player.getSkillTree().getTotalIceDamage();
                int damage = baseDamage + damageBonus;

                enemy->takeDamage(damage);
                enemy->applyIceStack(); // Apply stacking slow/freeze mechanic
                baseProjectile->setActive(false);
                auto levelUpInfo = player.gainExperience(10);
                player.applyLevelUpBonuses(levelUpInfo);

                sf::FloatRect enemyBounds = enemy->getCollisionBounds();
                if (floatingTextCallback)
                {
                    floatingTextCallback(damage,
                                         {enemyBounds.left + enemyBounds.width / 2.f, enemyBounds.top},
                                         sf::Color(100, 200, 255),
                                         false);
                }

                if (combatLogCallback)
                {
                    int stacks = enemy->getIceStacks();
                    std::string statusMsg = stacks >= 2 ? " (FROZEN!)" : " (SLOWED x" + std::to_string(stacks) + ")";
                    combatLogCallback("Ice Projectile hit! Dealt " + std::to_string(damage) + " damage to Enemy" + statusMsg);
                }

                // Handle enemy death from ice projectile
                if (!enemy->isAlive())
                {
                    const sf::FloatRect enemyDeathBounds = enemy->getCollisionBounds();
                    spawnPotion(hpPotions, MAX_HP_POTIONS, resourceManager.getTexture("hp_potion"),
                                {enemyDeathBounds.left + enemyDeathBounds.width / 2.f - 18.f,
                                 enemyDeathBounds.top + enemyDeathBounds.height - 56.f});
                    auto levelUpInfo = player.gainExperience(25);
                    player.applyLevelUpBonuses(levelUpInfo);

                    player.getSkillTree().addSkillPoints(1);

                    // Deactivate the enemy so it's no longer rendered or updated
                    enemy->setActive(false);

                    if (combatLogCallback)
                    {
                        combatLogCallback("Defeated Enemy! (+25 XP, +1 Skill Point)");
                    }
                }
                break;
            }
        }

        // Collision with boss
        if (projectile && projectile->isActive() && bossSpawned && boss && boss->isAlive() && checkRectIntersection(projectileBounds, boss->getCollisionBounds()))
        {
            int baseDamage = projectile->getDamage();
            int damageBonus = player.getSkillTree().getTotalIceDamage();
            int damage = baseDamage + damageBonus;

            boss->takeDamage(damage);
            boss->applyIceStack();
            baseProjectile->setActive(false);
            auto levelUpInfo = player.gainExperience(20);
            player.applyLevelUpBonuses(levelUpInfo);

            sf::FloatRect bossBounds = boss->getCollisionBounds();
            if (floatingTextCallback)
            {
                floatingTextCallback(damage,
                                     {bossBounds.left + bossBounds.width / 2.f, bossBounds.top},
                                     sf::Color(100, 200, 255),
                                     false);
            }

            if (combatLogCallback)
            {
                int stacks = boss->getIceStacks();
                std::string statusMsg = stacks >= 2 ? " (FROZEN!)" : " (SLOWED x" + std::to_string(stacks) + ")";
                combatLogCallback("Ice Projectile hit! Dealt " + std::to_string(damage) + " damage to BOSS!" + statusMsg);
            }

            // If boss dies from ice projectile, trigger slow motion victory
            if (!boss->isAlive())
            {
                auto levelUpInfo = player.gainExperience(100);
                player.applyLevelUpBonuses(levelUpInfo);
                triggerBossDefeated(bossDeathSlowMotionActive,
                                    bossDeathSlowMotionTimer,
                                    slowMotionDuration,
                                    "Boss defeated by ice projectile! Starting slow motion...",
                                    "*** BOSS DEFEATED WITH ICE PROJECTILE! Victory! (+100 XP) ***");
            }
        }
    }
}

// Updates world potions, handles pickup and reclaiming inactive potions
void CombatSystem::updatePotions(float dt, Player &player)
{
    const sf::FloatRect playerCollision = player.getCollisionBounds();

    // Iterate through all HP potions
    for (auto it = hpPotions.begin(); it != hpPotions.end();)
    {
        auto &potion = *it;
        HPPotion *hpPotion = potion.get();
        Entity *potionEntity = hpPotion;
        if (!potionEntity || !potionEntity->isActive())
        {
            ++it;
            continue;
        }

        // Update potion animation/behavior
        logPolymorphicDispatch(potionEntity, "Entity", "update(float)");
        potionEntity->update(dt);

        // Check collision between player and potion
        if (checkRectIntersection(playerCollision, potionEntity->getBounds()))
        {
            // If player is not at full HP, auto-heal
            if (player.getHp() < player.getMaxHp())
            {
                int healAmount = hpPotion->getHealingValue();
                int newHp = player.getHp() + healAmount;
                player.setHp(newHp);
                potionEntity->setActive(false);

                if (floatingTextCallback)
                {
                    floatingTextCallback(healAmount,
                                         {playerCollision.left + playerCollision.width / 2.f, playerCollision.top},
                                         sf::Color::Green,
                                         true);
                }
            }
            // Otherwise, add potion to inventory if possible
            else if (player.addHpPotion())
            {
                potionEntity->setActive(false);
            }
        }
        ++it;
    }

    // Move inactive potions back to pool
    reclaimInactive(hpPotions, hpPotionPool, "Potions");
}

// Updates meteor projectiles, spawns meteor showers, and handles meteor damage to the player
void CombatSystem::updateMeteors(float dt,
                                 Player &player,
                                 float groundLevel,
                                 const sf::View &camera,
                                 std::mt19937 &rng,
                                 const GameMaster *gameMaster)
{
    const sf::FloatRect playerCollision = player.getCollisionBounds();

    // Update each meteor and resolve collisions and ground impact
    static int meteorFrameCounter = 0;
    meteorFrameCounter++;

    int meteorIndex = 0;
    for (auto it = meteors.begin(); it != meteors.end();)
    {
        auto &meteor = *it;
        Projectile *baseMeteor = meteor.get();
        if (!baseMeteor || !baseMeteor->isActive())
        {
            ++it;
            continue;
        }

        if (ENABLE_POLYMORPHISM_STDOUT && meteorFrameCounter == 120 && meteorIndex == 0)
        {
            std::cout << "  - Meteor[" << meteorIndex << "]::update() called (falling)\n";
        }
        logPolymorphicDispatch(baseMeteor, "Projectile", "update(float)");
        baseMeteor->update(dt);
        meteorIndex++;

        const sf::FloatRect bounds = baseMeteor->getBounds();
        const float bottom = bounds.top + bounds.height;

        // Deactivate meteor when it hits the ground or goes out of world bounds
        if (bottom >= groundLevel || baseMeteor->getPosition().y > WORLD_HEIGHT + 200.f)
        {
            baseMeteor->setActive(false);
            ++it;
            continue;
        }

        // Check if meteor hits the player
        if (checkRectIntersection(bounds, playerCollision))
        {
            int damage = baseMeteor->getDamage();
            player.takeDamage(damage);
            baseMeteor->setActive(false);

            resourceManager.playGotHitSound();
            if (floatingTextCallback)
            {
                floatingTextCallback(damage,
                                     {playerCollision.left + playerCollision.width / 2.f, playerCollision.top},
                                     sf::Color::Magenta,
                                     false);
            }
            if (combatLogCallback)
            {
                combatLogCallback("Hit by Meteor! Took " + std::to_string(damage) + " damage");
            }
        }
        ++it;
    }

    // Reclaim inactive meteors to pool
    reclaimInactive(meteors, meteorPool, "Meteors");

    // Check if any active meteor exists for continuous meteor sound
    bool anyMeteorActive = false;
    for (auto it = meteors.begin(); it != meteors.end(); ++it)
    {
        auto &meteor = *it;
        Projectile *meteorProjectile = meteor.get();
        if (meteorProjectile && meteorProjectile->isActive())
        {
            anyMeteorActive = true;
            break;
        }
    }

    const bool sandStormActive = (gameMaster && gameMaster->isSandStormActive());

    // Play or stop meteor ambient sound based on activity, but mute during sand storms
    if (sandStormActive)
    {
        resourceManager.stopMeteorSound();
    }
    else if (anyMeteorActive)
    {
        resourceManager.playMeteorSound();
    }
    else
    {
        resourceManager.stopMeteorSound();
    }

    // Track whether Meteor Fury mode just got activated this frame
    static bool wasMeteorFuryActive = false;
    bool isMeteorFuryActive = (gameMaster && gameMaster->isMeteorFuryActive());

    // If meteor fury just turned on, force a shower soon
    if (isMeteorFuryActive && !wasMeteorFuryActive)
    {
        // Skip initial long delay by setting timer to threshold
        meteorShowerTimer = nextMeteorShower;
    }
    wasMeteorFuryActive = isMeteorFuryActive;

    // Increment meteor shower timer
    meteorShowerTimer += dt;
    if (meteorShowerTimer >= nextMeteorShower)
    {
        // Reset timer when it's time to spawn a new shower
        meteorShowerTimer = 0.f;

        // Default meteor settings
        float minInterval = 10.f;
        float maxInterval = 20.f;
        int meteorCount = 3;
        int maxMeteorCount = 6;
        float playerTargetRatio = 0.3f;

        // Increase intensity during Meteor Fury
        if (isMeteorFuryActive)
        {
            minInterval = 1.f;
            maxInterval = 2.f;
            meteorCount = 20;
            maxMeteorCount = 30;
            playerTargetRatio = 0.5f;
        }

        // Randomize next meteor shower interval
        std::uniform_real_distribution<float> meteorInterval(minInterval, maxInterval);
        nextMeteorShower = meteorInterval(rng);

        // Compute spawn area above the camera view
        const sf::Vector2f center = camera.getCenter();
        const sf::FloatRect spawnRect(center.x - SCREEN_WIDTH / 2.f, -120.f, SCREEN_WIDTH, 0.f);
        std::uniform_int_distribution<int> meteorCountDistribution(meteorCount, maxMeteorCount);

        // Spawn meteor burst with some chance to target the player's x-position
        spawnMeteorBurst(meteors, MAX_METEORS, resourceManager, spawnRect, rng,
                         meteorCountDistribution(rng), player.getPosition(), playerTargetRatio);
    }
}

// Spawns an HP potion at a specific world position using pooling when possible
void CombatSystem::spawnPotion(DoublyLinkedList<std::unique_ptr<HPPotion>> &potions,
                               std::size_t maxPotions,
                               const sf::Texture &texture,
                               sf::Vector2f worldPos)
{
    // Reclaim any inactive potions before spawning new ones
    reclaimInactive(potions, hpPotionPool, "Potions");

    // Do not exceed maximum allowed potions
    if (maxPotions == 0 || potions.size() >= maxPotions)
        return;

    std::unique_ptr<HPPotion> potion;
    // Try to reuse a potion from the pool
    if (!hpPotionPool.isEmpty())
    {
        potion = std::move(hpPotionPool.front());
        hpPotionPool.popFront();
        if (potion)
        {
            potion->reset(texture, worldPos);
        }
    }
    else
    {
        // Allocate a new potion if pool is empty
        potion = std::make_unique<HPPotion>(texture, worldPos);
    }

    // Add potion to the active list if valid
    if (potion)
    {
        potions.pushBack(std::move(potion));
        if (ENABLE_DOUBLY_LINKED_LIST_STDOUT)
        {
            logListState("HP Potions", potions, hpPotionPool);
            std::cout << "    action: spawned 1 HP potion into active list" << std::endl;
        }
    }
}

// Spawns a burst of meteors within the given spawn bounds
void CombatSystem::spawnMeteorBurst(DoublyLinkedList<std::unique_ptr<Meteor>> &meteors,
                                    std::size_t maxMeteors,
                                    ResourceManager &resourceManager,
                                    const sf::FloatRect &spawnBounds,
                                    std::mt19937 &rng,
                                    int count,
                                    sf::Vector2f playerPos,
                                    float playerTargetRatio)
{
    // If requested count is non-positive, nothing to spawn
    if (count <= 0)
        return;

    // Reclaim inactive meteors before spawning new ones
    reclaimInactive(meteors, meteorPool, "Meteors");

    const std::size_t currentCount = meteors.size();
    if (maxMeteors == 0 || currentCount >= maxMeteors)
        return;

    // Clamp spawn count so we don't exceed maxMeteors
    const std::size_t spawnCount = std::min<std::size_t>(static_cast<std::size_t>(count), maxMeteors - currentCount);

    // Cache meteor textures from resource manager
    sf::Texture *meteorTextures[5]{};
    resourceManager.getMeteorTextures(meteorTextures);

    const float minX = spawnBounds.left;
    const float maxX = spawnBounds.left + spawnBounds.width;
    std::uniform_real_distribution<float> horizontal(minX, maxX);
    std::uniform_real_distribution<float> playerOffset(-64.f, 64.f);
    std::uniform_real_distribution<float> probability(0.f, 1.f);

    // Spawn each meteor
    std::size_t spawned = 0;
    for (std::size_t i = 0; i < spawnCount; ++i)
    {
        sf::Vector2f spawnPos{spawnBounds.left, spawnBounds.top};
        const bool targetPlayer = probability(rng) < playerTargetRatio;
        if (targetPlayer)
        {
            // Spawn closer to the player's x-position
            spawnPos.x = std::clamp(playerPos.x + playerOffset(rng), minX, maxX);
        }
        else
        {
            // Spawn at random x within the sub-rect
            spawnPos.x = horizontal(rng);
        }

        std::unique_ptr<Meteor> meteor;
        // Use meteor from pool if available
        if (!meteorPool.isEmpty())
        {
            meteor = std::move(meteorPool.front());
            meteorPool.popFront();
            if (meteor)
            {
                meteor->reset(meteorTextures, spawnPos);
            }
        }
        else
        {
            // Otherwise, create a fresh meteor instance
            meteor = std::make_unique<Meteor>(meteorTextures, spawnPos);
        }

        // Add meteor to active list if valid
        if (meteor)
        {
            if (ENABLE_DOUBLY_LINKED_LIST_STDOUT)
            {
                std::cout << "[DoublyLinkedList] meteor node @" << meteor.get() << std::endl;
            }
            meteors.pushBack(std::move(meteor));
            ++spawned;
        }
    }

    if (spawned > 0 && ENABLE_DOUBLY_LINKED_LIST_STDOUT)
    {
        logListState("Meteors", meteors, meteorPool);
        std::cout << "    action: spawned " << spawned << " meteor(s)" << std::endl;

        if (ENABLE_ITERATOR_STDOUT)
            std::cout << "[Iterator] DoublyLinkedList iterator operations used for meteor spawn management" << std::endl;
    }
}

// Returns current projectile ammo count
int CombatSystem::getProjectileAmmo() const
{
    return projectileAmmo;
}

// Returns current projectile cooldown time
float CombatSystem::getProjectileCooldown() const
{
    return projectileCooldown;
}

// Returns maximum projectile ammo based on player's skill tree
int CombatSystem::getMaxProjectileAmmo(const Player &player) const
{
    // Return fire or ice projectile charges based on current type
    if (currentProjectileType == ProjectileType::FIRE)
    {
        return player.getSkillTree().getFireProjectileCharges();
    }
    else
    {
        return player.getSkillTree().getIceProjectileCharges();
    }
}

// Returns current special attack ammo count
int CombatSystem::getSpecialAttackAmmo() const
{
    return specialAttackAmmo;
}

// Returns current special attack cooldown time
float CombatSystem::getSpecialAttackCooldown() const
{
    return specialAttackCooldown;
}

// Returns whether fire projectile type is currently selected
bool CombatSystem::isUsingFireProjectile() const
{
    return currentProjectileType == ProjectileType::FIRE;
}

// Returns the effective projectile type based on player's unlocked skills
bool CombatSystem::isEffectivelyUsingFireProjectile(const Player &player) const
{
    // Check which projectile types player has unlocked
    bool hasFire = player.getSkillTree().hasFireProjectile();
    bool hasIce = player.getSkillTree().hasIceProjectile();

    // Determine effective type based on unlocks and current selection
    ProjectileType effective = currentProjectileType;
    if (effective == ProjectileType::FIRE && !hasFire && hasIce)
        effective = ProjectileType::ICE;
    else if (effective == ProjectileType::ICE && !hasIce && hasFire)
        effective = ProjectileType::FIRE;

    return effective == ProjectileType::FIRE;
}

// Returns non-const reference to fire projectile list
DoublyLinkedList<std::unique_ptr<FireProjectile>> &CombatSystem::getFireProjectiles()
{
    return fireProjectiles;
}

// Returns non-const reference to ice projectile list
DoublyLinkedList<std::unique_ptr<IceProjectile>> &CombatSystem::getIceProjectiles()
{
    return iceProjectiles;
}

// Returns non-const reference to HP potion list
DoublyLinkedList<std::unique_ptr<HPPotion>> &CombatSystem::getHPPotions()
{
    return hpPotions;
}

// Returns non-const reference to meteor list
DoublyLinkedList<std::unique_ptr<Meteor>> &CombatSystem::getMeteors()
{
    return meteors;
}

// Triggers boss defeat sequence with slow motion effect
void CombatSystem::triggerBossDefeated(bool &bossDeathSlowMotionActive,
                                       float &bossDeathSlowMotionTimer,
                                       float slowMotionDuration,
                                       const char *debugMessage,
                                       const std::string &logMessage)
{
    // Only trigger if slow motion is not already active
    if (!bossDeathSlowMotionActive)
    {
        // Activate slow motion and set timer
        bossDeathSlowMotionActive = true;
        bossDeathSlowMotionTimer = slowMotionDuration;
        // Push boss death slow motion state
        if (pushStateCallback)
        {
            pushStateCallback(GameStateType::BossDeathSlowMotion);
        }
        // Log victory message
        if (combatLogCallback)
        {
            combatLogCallback(logMessage);
        }
        // debugMessage is currently unused; it can be logged or used for debugging if needed
    }
}
