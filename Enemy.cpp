#include "Enemy.h"
#include "Constants.h"
#include <algorithm>
#include <cmath>

// Constructor that initializes the enemy at a position with patrol boundaries and animation
Enemy::Enemy(float x, float y, float leftBound, float rightBound,
             const EnemyAnimation &anim)
    : Character(), animation(anim)
{
    // Initialize sprite with run texture
    sprite.setTexture(animation.runTexture);
    // Set initial position
    position = {x, y};
    sprite.setPosition(position);
    // Set patrol boundaries
    patrolLeft = leftBound;
    patrolRight = rightBound;
    // Initialize movement and facing direction
    movingRight = true;
    facingRight = true;
    // Initialize animation state
    currentFrame = 0;
    animationTimer = 0.f;
    // Initialize physics
    velocityY = 0.f;
    // Initialize AI state
    state = EnemyState::PATROLLING;
    chaseSpeed = ENEMY_CHASE_SPEED;
    attackCooldown = 0.f;
    detectionRange = ENEMY_DETECTION_RANGE;
    attackRange = ENEMY_ATTACK_RANGE;
    isAttacking = false;
    attackAnimationTimer = 0.f;
    attackFrame = 0;
    active = true;

    // Initialize status effects
    iceStacks = 0;
    iceStackDuration = 0.f;

    isBurning = false;
    burnDamage = 0;
    burnDuration = 0.f;
    burnTickTimer = 0.f;

    // Initialize combat stats
    initializeCombatStats(ENEMY_STARTING_HP, ENEMY_STARTING_DAMAGE);
    speed = ENEMY_MOVEMENT_SPEED;
    setOnGround(false);
}

// Updates ice stack duration and removes stacks when they expire
void Enemy::tickIceStacks(float dt)
{
    // Only process if enemy has ice stacks
    if (iceStacks > 0)
    {
        // Decrement ice stack duration
        iceStackDuration -= dt;
        // Remove ice stacks when duration expires
        if (iceStackDuration <= 0.f)
        {
            iceStacks = 0;
            iceStackDuration = 0.f;
        }
    }
}

// Override of base update method (empty implementation, actual update is in overloaded version)
void Enemy::update(float dt)
{
}

// Updates the enemy state, movement, and animations based on player position
void Enemy::update(float dt, float gravity, const Player &player)
{
    // Return early if enemy is dead
    if (!isAlive())
        return;

    // Update ice stack effects
    tickIceStacks(dt);

    // If frozen, show frozen frame and stop all movement
    if (isFrozen())
    {
        // Set sprite to run texture with first frame (frozen appearance)
        sprite.setTexture(animation.runTexture);
        sprite.setTextureRect(animation.runFrameRect(0));

        return;
    }

    // Get ground state and slow multiplier
    const bool grounded = getOnGround();
    float slowMult = getSlowMultiplier();

    // Handle attack animation
    if (isAttacking)
    {
        // Set sprite to attack texture
        sprite.setTexture(animation.attackTexture);
        attackAnimationTimer += dt;
        // Advance attack frame at specified frame rate
        if (attackAnimationTimer >= ENEMY_ANIMATION_FRAME_TIME)
        {
            attackFrame++;
            attackAnimationTimer = 0.f;
            // End attack when all frames are played
            if (attackFrame >= animation.attackFrames)
            {
                isAttacking = false;
                attackFrame = 0;
                sprite.setTexture(animation.idleTexture);
            }
        }
        sprite.setTextureRect(animation.attackFrameRect(attackFrame));
    }
    // Handle idle and run animations
    else
    {
        // Show idle animation when in attacking state but not attacking
        if (state == EnemyState::ATTACKING)
        {
            sprite.setTexture(animation.idleTexture);
            animationTimer += dt;
            // Animate idle with 0.15 second frame rate
            if (animationTimer >= 0.15f)
            {
                currentFrame = (currentFrame + 1) % animation.idleFrames;
                animationTimer = 0.f;
            }
            sprite.setTextureRect(animation.idleFrameRect(currentFrame));
        }
        // Show run animation when patrolling or chasing
        else
        {
            sprite.setTexture(animation.runTexture);
            animationTimer += dt;
            // Animate run with 0.15 second frame rate
            if (animationTimer >= 0.15f)
            {
                currentFrame = (currentFrame + 1) % animation.runFrames;
                animationTimer = 0.f;
            }
            sprite.setTextureRect(animation.runFrameRect(currentFrame));
        }
    }

    // Apply gravity if not grounded
    if (!grounded)
    {
        velocityY += gravity * dt;
    }

    // Update attack cooldown timer
    if (attackCooldown > 0.f)
    {
        attackCooldown -= dt;
    }

    // Get collision bounds for enemy and player
    sf::FloatRect enemyBounds = getCollisionBounds();
    sf::FloatRect playerBounds = player.getCollisionBounds();

    // Calculate horizontal distance to player
    float distanceToPlayer = std::abs((enemyBounds.left + enemyBounds.width / 2.f) -
                                      (playerBounds.left + playerBounds.width / 2.f));

    // State machine for enemy behavior
    switch (state)
    {
    case EnemyState::PATROLLING:
    {
        // Check if player is within detection range and vertically aligned
        if (distanceToPlayer <= detectionRange &&
            std::abs(enemyBounds.top - playerBounds.top) < 100.f)
        {
            // Transition to chasing state
            state = EnemyState::CHASING;
        }
        else
        {
            // Continue patrolling between boundaries
            if (grounded)
            {
                // Move right if moving right
                if (movingRight)
                {
                    position.x += speed * slowMult * dt;
                    // Calculate collision width and check if reached right boundary
                    float collisionWidth = animation.frameSize.x - (2 * LANCER_HORIZONTAL_MARGIN);
                    if (position.x + LANCER_HORIZONTAL_MARGIN + collisionWidth >= patrolRight)
                    {
                        // Turn around at right boundary
                        movingRight = false;
                        facingRight = false;
                    }
                }
                // Move left if moving left
                else
                {
                    position.x -= speed * slowMult * dt;
                    // Check if reached left boundary
                    if (position.x + LANCER_HORIZONTAL_MARGIN <= patrolLeft)
                    {
                        // Turn around at left boundary
                        movingRight = true;
                        facingRight = true;
                    }
                }
            }
        }
        break;
    }

    case EnemyState::CHASING:
    {
        float verticalDistance = std::abs(enemyBounds.top - playerBounds.top);

        // Return to patrolling if player is too far away or not vertically aligned
        if (distanceToPlayer > detectionRange * 1.5f || verticalDistance > 100.f)
        {
            state = EnemyState::PATROLLING;
        }
        // Transition to attacking if player is within attack range
        else if (distanceToPlayer <= attackRange * 0.7f && verticalDistance <= 80.f)
        {
            state = EnemyState::ATTACKING;
        }
        // Chase player if grounded and vertically aligned
        else if (grounded && verticalDistance <= 100.f)
        {
            // Calculate center positions for direction determination
            float playerCenter = playerBounds.left + playerBounds.width / 2.f;
            float enemyCenter = enemyBounds.left + enemyBounds.width / 2.f;

            // Move towards player
            if (playerCenter > enemyCenter)
            {
                position.x += chaseSpeed * slowMult * dt;
                facingRight = true;
                movingRight = true;
            }
            else
            {
                position.x -= chaseSpeed * slowMult * dt;
                facingRight = false;
                movingRight = false;
            }
        }
        break;
    }

    case EnemyState::ATTACKING:
    {
        float verticalDistance = std::abs(enemyBounds.top - playerBounds.top);

        // Return to chasing if player moves too far away
        if (distanceToPlayer > attackRange * 1.5f)
        {
            state = EnemyState::CHASING;
        }
        // Return to patrolling if player is too far or not aligned
        else if (distanceToPlayer > detectionRange * 1.5f || verticalDistance > 80.f)
        {
            state = EnemyState::PATROLLING;
        }
        else
        {
            // Start attack if cooldown is ready and player is aligned
            if (grounded && attackCooldown <= 0.f && !isAttacking && verticalDistance <= 50.f)
            {
                isAttacking = true;
                attackFrame = 0;
                attackAnimationTimer = 0.f;
                // Set cooldown adjusted by slow multiplier
                attackCooldown = 2.0f * (1.0f / slowMult);
            }

            // Calculate center positions for facing direction
            float playerCenter = playerBounds.left + playerBounds.width / 2.f;
            float enemyCenter = enemyBounds.left + enemyBounds.width / 2.f;

            // Maintain optimal attack distance
            if (distanceToPlayer < attackRange * 0.5f)
            {
                // Move away from player if too close
                if (playerCenter > enemyCenter)
                {
                    position.x -= speed * 0.5f * dt;
                    facingRight = true;
                }
                else
                {
                    position.x += speed * 0.5f * dt;
                    facingRight = false;
                }
            }
            else
            {
                // Just face the player if at optimal distance
                facingRight = (playerCenter > enemyCenter);
            }
        }
        break;
    }
    }

    // Update vertical position based on velocity
    position.y += velocityY * dt;
    sprite.setPosition(position);

    // Set sprite direction based on facing direction
    setSpriteDirection(sprite, facingRight, animation.frameSize);
}

// Returns the global bounds of the enemy sprite
sf::FloatRect Enemy::getBounds() const
{
    return sprite.getGlobalBounds();
}

// Returns the collision bounds of the enemy (smaller than visual bounds for tighter collision)
sf::FloatRect Enemy::getCollisionBounds() const
{
    // Get sprite bounds and apply margins to create smaller collision box
    sf::FloatRect bounds = sprite.getGlobalBounds();
    return sf::FloatRect(bounds.left + LANCER_HORIZONTAL_MARGIN, bounds.top + LANCER_VERTICAL_MARGIN,
                         bounds.width - (2 * LANCER_HORIZONTAL_MARGIN),
                         bounds.height - (2 * LANCER_VERTICAL_MARGIN));
}

// Returns the attack range bounds of the enemy
sf::FloatRect Enemy::getAttackBounds() const
{
    // Start with collision bounds
    sf::FloatRect bounds = getCollisionBounds();
    const float reach = attackRange;
    // Extend bounds in facing direction by attack range
    if (facingRight)
    {
        bounds.width += reach;
    }
    else
    {
        bounds.left -= reach;
        bounds.width += reach;
    }

    // Trim vertical bounds to create a more focused attack area
    const float verticalTrim = bounds.height * 0.2f;
    bounds.top += verticalTrim;
    bounds.height = std::max(0.f, bounds.height - verticalTrim * 2.f);
    return bounds;
}

// Returns the current state of the enemy
EnemyState Enemy::getState() const { return state; }
// Returns whether the enemy is currently attacking
bool Enemy::getIsAttacking() const { return isAttacking; }
// Returns the current attack animation frame
int Enemy::getAttackFrame() const { return attackFrame; }
// Returns the vertical velocity
float Enemy::getVelocityY() const { return velocityY; }
// Returns the left boundary of patrol area
float Enemy::getPatrolLeft() const { return patrolLeft; }
// Returns the right boundary of patrol area
float Enemy::getPatrolRight() const { return patrolRight; }
// Returns whether the enemy is moving right
bool Enemy::getMovingRight() const { return movingRight; }
// Returns the enemy animation data
const EnemyAnimation &Enemy::getAnimation() const { return animation; }
// Returns whether the enemy sprite is facing right
bool Enemy::isFacingRight() const { return facingRight; }
// Returns the attack range of the enemy
float Enemy::getAttackRange() const { return attackRange; }

// Sets the vertical velocity
void Enemy::setVelocityY(float vy) { velocityY = vy; }

// Checks if the enemy is frozen (has enough ice stacks)
bool Enemy::isFrozen() const
{
    return iceStacks >= FREEZE_THRESHOLD;
}

// Applies an ice stack to the enemy (slows or freezes)
void Enemy::applyIceStack()
{
    // Increment ice stacks and reset duration
    iceStacks++;
    iceStackDuration = ICE_STACK_LIFETIME;

    // Cap ice stacks at freeze threshold
    if (iceStacks > FREEZE_THRESHOLD)
    {
        iceStacks = FREEZE_THRESHOLD;
    }
}

// Returns the speed multiplier based on ice stacks (0.0 if frozen, 0.5 if slowed, 1.0 if normal)
float Enemy::getSlowMultiplier() const
{
    // Return 0.0 if frozen (completely stopped)
    if (isFrozen())
    {
        return 0.0f;
    }
    // Return 0.5 if has one ice stack (50% speed)
    else if (iceStacks == 1)
    {
        return 0.5f;
    }
    // Return 1.0 if no ice stacks (normal speed)
    return 1.0f;
}

// Applies burn effect to the enemy
void Enemy::applyBurn(int damage, float duration)
{
    isBurning = true;
    burnDamage = damage;
    burnDuration = duration;
    burnTickTimer = BURN_TICK_INTERVAL;
}

// Updates burn effect and applies damage (without floating text)
void Enemy::updateBurnEffect(float dt)
{
    updateBurnEffect(dt, nullptr);
}

// Updates burn effect and applies damage (with optional floating text callback)
void Enemy::updateBurnEffect(float dt, std::function<void(int, sf::Vector2f, const sf::Color &, bool)> floatingTextCallback)
{
    // Return early if not burning or dead
    if (!isBurning || !isAlive())
    {
        return;
    }

    // Decrement burn duration
    burnDuration -= dt;
    // Remove burn effect when duration expires
    if (burnDuration <= 0.f)
    {
        isBurning = false;
        burnDamage = 0;
        burnDuration = 0.f;
        burnTickTimer = 0.f;
        return;
    }

    // Update burn tick timer
    burnTickTimer -= dt;
    // Apply burn damage when tick timer expires
    if (burnTickTimer <= 0.f)
    {
        // Apply burn damage to enemy
        takeDamage(burnDamage);

        // Display floating damage text if callback is provided
        if (floatingTextCallback)
        {
            sf::FloatRect bounds = getCollisionBounds();
            floatingTextCallback(burnDamage,
                                 sf::Vector2f(bounds.left + bounds.width / 2.f, bounds.top),
                                 sf::Color(255, 100, 0),
                                 false);
        }

        // Reset burn tick timer
        burnTickTimer = BURN_TICK_INTERVAL;
    }
}

// Returns the number of ice stacks on the enemy
int Enemy::getIceStacks() const
{
    return iceStacks;
}

// Returns whether the enemy is currently burning
bool Enemy::getIsBurning() const
{
    return isBurning;
}
