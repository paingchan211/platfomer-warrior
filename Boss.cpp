#include "Boss.h"
#include "Constants.h"
#include <cmath>

// Returns a dummy animation for Enemy base class initialization (boss uses BossAnimation instead)
EnemyAnimation *Boss::getDummyAnimation()
{
    // Static dummy animation that is initialized once
    static EnemyAnimation dummy;
    static bool initialized = false;
    // Initialize dummy animation with minimal valid data
    if (!initialized)
    {
        dummy.runFrames = 1;
        dummy.attackFrames = 1;
        dummy.frameSize = {1u, 1u};
        // Create minimal 1x1 textures for the dummy animation
        dummy.runTexture.create(1, 1);
        dummy.attackTexture.create(1, 1);
        initialized = true;
    }
    return &dummy;
}

// Constructor that initializes the boss with boss animation data
Boss::Boss(const BossAnimation &bossAnim)
    : Enemy(0.f, 0.f, 0.f, WORLD_WIDTH, *getDummyAnimation()),
      bossAnimation(bossAnim),
      bossState(BossState::INACTIVE),
      spawnTimer(0.f),
      isRegenerating(false),
      hasRegeneratedOnce(false),
      regenerationRate(100.0f),
      accumulatedHealing(0.0f)
{
    // Initialize combat stats with boss-specific values
    initializeCombatStats(BOSS_STARTING_HP, BOSS_STARTING_DAMAGE);
    speed = BOSS_MOVEMENT_SPEED;
    chaseSpeed = BOSS_CHASE_SPEED;
    setOnGround(false);

    // Initialize animation and combat timers
    currentFrame = 0;
    animationTimer = 0.f;
    attackCooldown = 0.f;
    detectionRange = BOSS_DETECTION_RANGE;
    attackRange = BOSS_ATTACK_RANGE;
    isAttacking = false;
    attackAnimationTimer = 0.f;
    attackFrame = 0;

    // Initialize position and velocity, set boss as inactive
    position = {0.f, 0.f};
    velocity = {0.f, 0.f};
    setActive(false);

    // Set up the sprite with boss animation texture
    sprite.setTexture(bossAnimation.texture);
    sprite.setTextureRect(bossAnimation.frameRect(0, 0));
    sprite.setScale({BOSS_SPRITE_SCALE, BOSS_SPRITE_SCALE});

    // Boss initially faces right
    facingRight = true;
}

// Spawns the boss at the specified position
void Boss::spawn(sf::Vector2f spawnPos)
{
    // Set boss position and update sprite position
    position = spawnPos;
    sprite.setPosition(position);
    // Set boss to spawning state with 2 second spawn timer
    bossState = BossState::SPAWNING;
    spawnTimer = 2.0f;
    setActive(true);
    // Reset velocity and restore full health
    velocity = {0.f, 0.f};
    setHp(maxHp);
    // Reset regeneration state
    isRegenerating = false;
    hasRegeneratedOnce = false;
    accumulatedHealing = 0.0f;
}

// Updates the boss state, movement, and animations based on player position
void Boss::update(float dt, float gravity, const Player &player, bool isEnraged)
{
    // Early return if boss is not active or inactive
    if (!isActive() || bossState == BossState::INACTIVE)
        return;

    // Update ice stack effects
    tickIceStacks(dt);

    // If frozen, stop movement and show frozen frame
    if (isFrozen())
    {
        velocity.x = 0.f;
        sprite.setTextureRect(bossAnimation.frameRect(0, 0));
        return;
    }

    // Check if boss should start regenerating health (below 30% HP, only once)
    float healthPercentage = static_cast<float>(hp) / static_cast<float>(maxHp);
    if (healthPercentage < 0.30f && !isRegenerating && !hasRegeneratedOnce)
    {
        isRegenerating = true;
        accumulatedHealing = 0.0f;
    }

    // Regenerate health if regeneration is active
    if (isRegenerating && hp < maxHp)
    {
        // Accumulate fractional healing points
        accumulatedHealing += regenerationRate * dt;
        // Apply whole number healing when accumulated healing >= 1
        if (accumulatedHealing >= 1.0f)
        {
            int healAmount = static_cast<int>(accumulatedHealing);
            hp += healAmount;
            accumulatedHealing -= healAmount;
        }

        // Stop regeneration when health is full
        if (hp >= maxHp)
        {
            hp = maxHp;
            isRegenerating = false;
            hasRegeneratedOnce = true;
            accumulatedHealing = 0.0f;
        }
    }
    // Get ground state and slow multiplier
    const bool grounded = getOnGround();
    float slowMult = getSlowMultiplier();

    // Calculate rage multipliers for speed and attack speed
    float rageSpeedMult = isEnraged ? BOSS_RAGE_SPEED_MULTIPLIER : 1.0f;
    float rageAttackSpeedMult = isEnraged ? BOSS_RAGE_ATTACK_SPEED_MULTIPLIER : 1.0f;

    // Update animation timer
    animationTimer += dt;

    // Get collision bounds for boss and player
    sf::FloatRect bossCollisionBounds = getCollisionBounds();
    sf::FloatRect playerBounds = player.getCollisionBounds();

    // Calculate center positions of boss and player
    float bossCenterX = bossCollisionBounds.left + bossCollisionBounds.width / 2.f;
    float bossCenterY = bossCollisionBounds.top + bossCollisionBounds.height / 2.f;
    float playerCenterX = playerBounds.left + playerBounds.width / 2.f;
    float playerCenterY = playerBounds.top + playerBounds.height / 2.f;
    // Calculate distances and check if vertically aligned
    float horizontalDistance = std::abs(playerCenterX - bossCenterX);
    float verticalDistance = std::abs(playerCenterY - bossCenterY);
    bool verticallyAligned = verticalDistance <= 120.f;

    // State machine for boss behavior
    switch (bossState)
    {
    case BossState::SPAWNING:
    {
        // Decrement spawn timer
        spawnTimer -= dt;
        // Transition to chasing state after spawn timer expires
        if (spawnTimer <= 0.f)
        {
            bossState = BossState::CHASING;
        }

        // Animate spawning with 0.2 second frame rate
        if (animationTimer >= 0.2f)
        {
            currentFrame = (currentFrame + 1) % bossAnimation.framesPerRow;
            animationTimer = 0.f;
        }
        // Use row 0 for spawning animation
        sprite.setTextureRect(bossAnimation.frameRect(currentFrame, 0));
        break;
    }

    case BossState::PATROLLING:
        // Patrolling state immediately transitions to chasing
        bossState = BossState::CHASING;
        break;

    case BossState::IDLE:
    case BossState::CHASING:
    {
        // Check if boss is directly under player (not vertically aligned but within attack range)
        bool bossDirectlyUnderPlayer = !verticallyAligned && (horizontalDistance <= attackRange);

        // If player is within attack range and vertically aligned, start attacking
        if (horizontalDistance <= attackRange && verticallyAligned)
        {
            bossState = BossState::ATTACKING;
        }
        // If boss is directly under player, stop and face the player
        else if (bossDirectlyUnderPlayer)
        {
            bossState = BossState::IDLE;
            velocity.x = 0.f;
            // Face the player
            facingRight = (playerCenterX > bossCenterX);
        }
        // Otherwise, chase the player
        else
        {
            // Set state to chasing
            bossState = BossState::CHASING;

            // Move towards player if grounded
            if (grounded)
            {
                // Move right if player is to the right
                if (playerCenterX > bossCenterX)
                {
                    velocity.x = chaseSpeed * slowMult * rageSpeedMult;
                    facingRight = true;
                }
                // Move left if player is to the left
                else
                {
                    velocity.x = -chaseSpeed * slowMult * rageSpeedMult;
                    facingRight = false;
                }
            }
        }

        // Stop horizontal movement if not grounded
        if (!grounded)
        {
            velocity.x = 0.f;
        }

        // Use row 1 for chasing animation, row 0 for idle
        int animRow = (bossState == BossState::CHASING) ? 1 : 0;
        // Animate with 0.15 second frame rate
        if (animationTimer >= 0.15f)
        {
            currentFrame = (currentFrame + 1) % bossAnimation.framesPerRow;
            animationTimer = 0.f;
        }
        sprite.setTextureRect(bossAnimation.frameRect(currentFrame, animRow));
        break;
    }

    case BossState::ATTACKING:
    {
        // Determine player position relative to boss
        bool playerOnRight = playerCenterX > bossCenterX;
        facingRight = playerOnRight;

        // If player is not vertically aligned and boss is not attacking, return to chasing
        if (!verticallyAligned && !isAttacking)
        {
            bossState = BossState::CHASING;
            break;
        }

        // If not attacking and player is too far away, return to chasing
        if (!isAttacking)
        {
            if (horizontalDistance > attackRange * 1.5f)
            {
                bossState = BossState::CHASING;
                break;
            }
        }

        // Define desired distance range for optimal attack position
        const float desiredMinDistance = attackRange * 0.6f;
        const float desiredMaxDistance = attackRange * 1.2f;

        // Adjust position to maintain optimal attack distance
        if (grounded && verticallyAligned)
        {
            // Move towards player if too far
            if (horizontalDistance > desiredMaxDistance)
            {
                velocity.x = (playerOnRight ? chaseSpeed : -chaseSpeed) * 0.5f * slowMult * rageSpeedMult;
            }
            // Move away from player if too close
            else if (horizontalDistance < desiredMinDistance)
            {
                velocity.x = (playerOnRight ? -speed : speed) * 0.6f * slowMult * rageSpeedMult;
            }
            // Stay in place if at optimal distance
            else
            {
                velocity.x = 0.f;
            }
        }

        // Start attack if cooldown is ready
        if (!isAttacking && attackCooldown <= 0.f)
        {
            isAttacking = true;
            attackFrame = 0;
            attackAnimationTimer = 0.f;
            attackCooldown = 2.0f;
            // Adjust cooldown based on slow multiplier and rage attack speed
            attackCooldown *= (1.0f / slowMult);
            attackCooldown *= (1.0f / rageAttackSpeedMult);
        }

        // Handle attack animation
        if (isAttacking)
        {
            // Update attack animation timer
            attackAnimationTimer += dt;
            // Advance attack frame every 0.15 seconds
            if (attackAnimationTimer >= 0.15f)
            {
                attackFrame++;
                attackAnimationTimer = 0.f;
                // End attack when all frames are played
                if (attackFrame >= bossAnimation.framesPerRow)
                {
                    isAttacking = false;
                    attackFrame = 0;
                    bossState = BossState::IDLE;
                }
            }
            // Use row 2 for attack animation
            sprite.setTextureRect(bossAnimation.frameRect(attackFrame, 2));
        }
        // Show idle animation while not attacking
        else
        {
            if (animationTimer >= 0.2f)
            {
                currentFrame = (currentFrame + 1) % bossAnimation.framesPerRow;
                animationTimer = 0.f;
            }
            // Use row 0 for idle animation
            sprite.setTextureRect(bossAnimation.frameRect(currentFrame, 0));
        }
        break;
    }

    default:
        break;
    }

    // Decrement attack cooldown timer
    if (attackCooldown > 0.f)
    {
        attackCooldown -= dt;
    }

    // Apply gravity if not grounded
    if (!grounded)
    {
        velocity.y += gravity * dt;
    }

    // Update position based on velocity
    position += velocity * dt;
    sprite.setPosition(position);

    // Set sprite direction based on facing direction
    setSpriteDirection(sprite, facingRight, bossAnimation.frameSize, 2.5f, 2.5f);
}

// Returns the global bounds of the boss sprite
sf::FloatRect Boss::getBounds() const
{
    return sprite.getGlobalBounds();
}

// Returns the collision bounds of the boss (smaller than visual bounds for tighter collision)
sf::FloatRect Boss::getCollisionBounds() const
{
    // Return empty bounds if boss is not active
    if (!isActive())
        return sf::FloatRect();

    // Get sprite bounds and apply margins to create smaller collision box
    sf::FloatRect bounds = sprite.getGlobalBounds();
    float horizontalMargin = 170.f;
    float verticalMargin = 170.f;
    // Return collision bounds with margins applied
    return sf::FloatRect(bounds.left + horizontalMargin, bounds.top + verticalMargin,
                         bounds.width - (2 * horizontalMargin), bounds.height - (2 * verticalMargin));
}

// Checks if the boss is still alive
bool Boss::isAlive() const
{
    return isActive() && Character::isAlive();
}

// Returns the current state of the boss
BossState Boss::getBossState() const { return bossState; }
// Returns whether the boss is currently attacking
bool Boss::getIsAttacking() const { return isAttacking; }
// Returns the current attack animation frame
int Boss::getAttackFrame() const { return attackFrame; }
// Returns the attack range of the boss
float Boss::getAttackRange() const { return attackRange; }
// Returns the boss animation data
const BossAnimation &Boss::getAnimation() const { return bossAnimation; }

// Sets the boss state to a new state
void Boss::setBossState(BossState newState) { bossState = newState; }

// Compute the bounding box of a boss attack based on player direction
sf::FloatRect Boss::computeAttackBounds(bool playerOnRight) const
{
    // Start from the boss's current collision box
    sf::FloatRect attackBounds = getCollisionBounds();

    // Validate box dimensions
    if (attackBounds.width <= 0.f || attackBounds.height <= 0.f)
        return attackBounds;

    // Extend the hitbox based on attack range and direction
    const float reach = getAttackRange();
    if (playerOnRight)
    {
        attackBounds.width += reach; // Extend rightward
    }
    else
    {
        attackBounds.left -= reach; // Extend leftward
        attackBounds.width += reach;
    }

    // Trim top and bottom to create a smaller vertical attack range
    const float verticalTrim = attackBounds.height * 0.2f;
    attackBounds.top += verticalTrim;
    attackBounds.height -= verticalTrim * 2.f;

    return attackBounds;
}
