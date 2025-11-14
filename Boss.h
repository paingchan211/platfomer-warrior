#pragma once
#include "Enemy.h"
#include "Animation.h"
#include "Player.h"
#include <functional>

// Enumeration representing the different states the boss can be in
enum class BossState
{
    INACTIVE,   // Boss is not active in the game world
    SPAWNING,   // Boss is spawning and cannot act yet
    IDLE,       // Boss is standing still, waiting
    PATROLLING, // Boss is patrolling (currently unused, transitions to CHASING)
    CHASING,    // Boss is chasing the player
    ATTACKING   // Boss is performing an attack
};

// Boss class that extends Enemy with special boss-specific behavior
class Boss : public Enemy
{
private:
    // Returns a dummy animation for Enemy base class initialization
    static EnemyAnimation *getDummyAnimation();

    BossAnimation bossAnimation; // Animation data for the boss
    BossState bossState;         // Current state of the boss
    float spawnTimer;            // Timer for spawn animation duration
    bool isRegenerating;         // Whether the boss is currently regenerating health
    bool hasRegeneratedOnce;     // Whether the boss has already regenerated once
    float regenerationRate;      // Health points regenerated per second
    float accumulatedHealing;    // Accumulated fractional healing points

public:
    // Constructor that initializes the boss with boss animation data
    explicit Boss(const BossAnimation &bossAnim);

    // Spawns the boss at the specified position
    void spawn(sf::Vector2f spawnPos);
    // Updates the boss state, movement, and animations based on player position
    void update(float dt, float gravity, const Player &player, bool isEnraged = false);
    // Returns the global bounds of the boss sprite
    sf::FloatRect getBounds() const override;
    // Returns the collision bounds of the boss (smaller than visual bounds)
    sf::FloatRect getCollisionBounds() const;

    // Checks if the boss is still alive
    bool isAlive() const override;

    // Returns the current state of the boss
    BossState getBossState() const;
    // Returns whether the boss is currently attacking
    bool getIsAttacking() const;
    // Returns the current attack animation frame
    int getAttackFrame() const;
    // Returns the attack range of the boss
    float getAttackRange() const;
    // Returns the boss animation data
    const BossAnimation &getAnimation() const;

    // Sets the boss state to a new state
    void setBossState(BossState newState);

    // Computes the area affected by a boss attack, depending on player position
    sf::FloatRect computeAttackBounds(bool playerOnRight) const;
};
