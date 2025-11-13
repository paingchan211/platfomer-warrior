#pragma once
#include "Character.h"
#include "Animation.h"
#include "Player.h"
#include <functional>

// Collision margin constants for enemy collision bounds
const float LANCER_HORIZONTAL_MARGIN = 120.f; // Horizontal margin for collision bounds in pixels
const float LANCER_VERTICAL_MARGIN = 125.f; // Vertical margin for collision bounds in pixels

// Enumeration representing the different states the enemy can be in
enum class EnemyState
{
    PATROLLING, // Enemy is patrolling between boundaries
    CHASING, // Enemy is chasing the player
    ATTACKING // Enemy is attacking the player
};

// Enemy class that extends Character with AI behavior and status effects
class Enemy : public Character
{
protected:
    float patrolLeft; // Left boundary of patrol area
    float patrolRight; // Right boundary of patrol area
    bool movingRight; // Whether the enemy is currently moving right
    bool facingRight; // Whether the enemy sprite is facing right

    EnemyAnimation animation; // Animation data for the enemy
    int currentFrame; // Current animation frame index
    float animationTimer; // Timer for animation frame updates

    float velocityY; // Vertical velocity for physics

    EnemyState state; // Current state of the enemy
    float chaseSpeed; // Speed when chasing the player
    float attackCooldown; // Cooldown timer between attacks
    float detectionRange; // Range at which enemy detects the player
    float attackRange; // Range at which enemy can attack
    bool isAttacking; // Whether the enemy is currently performing an attack
    float attackAnimationTimer; // Timer for attack animation frames
    int attackFrame; // Current attack animation frame index

    int iceStacks; // Number of ice stacks applied to the enemy
    float iceStackDuration; // Remaining duration of ice stack effect

    bool isBurning; // Whether the enemy is currently burning
    int burnDamage; // Damage per burn tick
    float burnDuration; // Remaining duration of burn effect
    float burnTickTimer; // Timer for burn damage ticks

protected:
    // Updates ice stack duration and removes stacks when they expire
    void tickIceStacks(float dt);

public:
    // Constructor that initializes the enemy at a position with patrol boundaries and animation
    Enemy(float x, float y, float leftBound, float rightBound,
          const EnemyAnimation &anim);

    // Override of base update method (empty implementation)
    void update(float dt) override;
    // Accepts a visitor for the visitor pattern
    void accept(EntityVisitor &visitor) const override;
    // Updates the enemy state, movement, and animations based on player position
    void update(float dt, float gravity, const Player &player);
    // Returns the global bounds of the enemy sprite
    sf::FloatRect getBounds() const override;
    // Returns the collision bounds of the enemy (smaller than visual bounds)
    sf::FloatRect getCollisionBounds() const;
    // Returns the attack range bounds of the enemy
    sf::FloatRect getAttackBounds() const;

    // Returns the current state of the enemy
    EnemyState getState() const;
    // Returns whether the enemy is currently attacking
    bool getIsAttacking() const;
    // Returns the current attack animation frame
    int getAttackFrame() const;
    // Returns the vertical velocity
    float getVelocityY() const;
    // Returns the left boundary of patrol area
    float getPatrolLeft() const;
    // Returns the right boundary of patrol area
    float getPatrolRight() const;
    // Returns whether the enemy is moving right
    bool getMovingRight() const;
    // Returns the enemy animation data
    const EnemyAnimation &getAnimation() const;
    // Returns whether the enemy sprite is facing right
    bool isFacingRight() const;
    // Returns the attack range of the enemy
    float getAttackRange() const;

    // Sets the vertical velocity
    void setVelocityY(float vy);

    // Applies an ice stack to the enemy (slows or freezes)
    void applyIceStack();
    // Returns the number of ice stacks on the enemy
    int getIceStacks() const;
    // Checks if the enemy is frozen (has enough ice stacks)
    bool isFrozen() const;
    // Returns the speed multiplier based on ice stacks (0.0 if frozen, 0.5 if slowed, 1.0 if normal)
    float getSlowMultiplier() const;

    // Applies burn effect to the enemy
    void applyBurn(int damage, float duration);
    // Returns whether the enemy is currently burning
    bool getIsBurning() const;
    // Updates burn effect and applies damage (without floating text)
    void updateBurnEffect(float dt);
    // Updates burn effect and applies damage (with optional floating text callback)
    void updateBurnEffect(float dt, std::function<void(int, sf::Vector2f, const sf::Color &, bool)> floatingTextCallback);
};
