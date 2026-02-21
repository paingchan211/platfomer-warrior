#include "Player.h"
#include "Platform.h"
#include "HPPotion.h"
#include "Constants.h"
#include "KeyBindingManager.h"
#include <iostream>

// Constructor initializes animations, stats, physics, and collision
Player::Player(Animation *run, Animation *idle, Animation *atk1, Animation *atk2,
               float startX, float startY, float charHeight, float grav, float jump)
    : Character(), runAnim(run), idleAnim(idle), attack1Anim(atk1), attack2Anim(atk2)
{
    // Initialize base combat stats
    initializeCombatStats(PLAYER_STARTING_HP, PLAYER_STARTING_DAMAGE);

    // Movement speed
    speed = PLAYER_MOVEMENT_SPEED;

    // Assign texture and position
    sprite.setTexture(run->texture);
    position = {startX, startY};
    sprite.setPosition(position);
    velocity = {0.f, 0.f};
    active = true;

    // Animation variables
    currentFrame = 0;
    animationTimer = 0.f;
    isRunning = false;
    facingRight = true;
    isAttacking = false;
    attackType = 0;

    // Special attack setup
    isSpecialAttacking = false;
    specialDashSpeed = PLAYER_SPECIAL_DASH_SPEED;
    specialDashDistance = PLAYER_SPECIAL_DASH_DISTANCE;
    specialDashTraveled = 0.f;

    // Speed boost modifier
    speedBoost = PLAYER_SPEED_BOOST_MULTIPLIER;

    // Jump and gravity setup
    gravity = grav;
    jumpStrength = jump;
    maxJumps = PLAYER_MAX_JUMPS;
    jumpCount = 0;
    spacePressed = false;

    // Collision box setup
    characterHeight = charHeight;
    margin = PLAYER_HORIZONTAL_MARGIN;
    collisionWidth = static_cast<float>(run->frameSize.x) - (2 * margin);
    collisionHeight = characterHeight - (2 * margin);
    collisionOffsetX = margin;
    collisionOffsetY = margin - PLAYER_GROUND_ADJUSTMENT;

    // Initialize skill tree system
    skillTree.initialize();

    // Base progression and inventory setup
    initializeProgressionStats();
    hpPotions = 0;
    maxHpPotions = 5;

    if (ENABLE_INHERITANCE_STDOUT)
    {
        std::cout << "[Inheritance] Player constructed -> Character -> Entity | "
                  << "pos=(" << position.x << ", " << position.y << "), "
                  << "level=" << level
                  << ", hp=" << getHp() << "/" << getMaxHp()
                  << ", damage=" << getDamage() << std::endl;
    }
}

// Update animation logic each frame
void Player::update(float dt)
{
    Animation *anim = nullptr;
    float currentFrameTime = 0.1f;

    // Determine current animation
    if (isAttacking || isSpecialAttacking)
    {
        if (attackType == 3)
        {
            anim = attack2Anim;
            currentFrameTime = 0.1f;
        }
        else
        {
            anim = (attackType == 1) ? attack1Anim : attack2Anim;
            currentFrameTime = 0.15f;
        }
    }
    else
    {
        anim = isRunning ? runAnim : idleAnim;
    }

    sprite.setTexture(anim->texture);

    // Handle animation frame timing
    animationTimer += dt;
    if (animationTimer >= currentFrameTime)
    {
        currentFrame++;
        animationTimer = 0.f;

        // Reset after attack animation completes
        if ((isAttacking || isSpecialAttacking) && currentFrame >= anim->frames)
        {
            isAttacking = false;
            isSpecialAttacking = false;
            attackType = 0;
            currentFrame = 0;
            specialDashTraveled = 0.f;
        }
        else
        {
            currentFrame = currentFrame % anim->frames;
        }
    }

    // Apply frame and direction
    sprite.setTextureRect(anim->frameRect(currentFrame));
    setSpriteDirection(sprite, facingRight, anim->frameSize);
}

// Handle user input for movement, sprinting, and dashing
void Player::handleInput(float dt, const KeyBindingManager *keyManager, bool sprintDisabled)
{
    // Handle special dash attack
    if (isSpecialAttacking)
    {
        float dashAmount = specialDashSpeed * dt;
        if (specialDashTraveled + dashAmount > specialDashDistance)
        {
            dashAmount = specialDashDistance - specialDashTraveled;
        }

        if (facingRight)
            move(dashAmount);
        else
            move(-dashAmount);

        specialDashTraveled += dashAmount;
        return;
    }

    // Prevent movement while attacking
    if (isAttacking)
        return;

    float moveSpeed = speed;

    // Sprinting using Shift key (disabled during certain events like sand storms)
    if (!sprintDisabled &&
        (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift)))
        moveSpeed *= speedBoost;

    bool moving = false;

    // Movement input (supports key binding manager)
    bool moveLeftPressed = keyManager ? sf::Keyboard::isKeyPressed(keyManager->getKeyForAction(GameAction::MOVE_LEFT))
                                      : sf::Keyboard::isKeyPressed(sf::Keyboard::Left);

    bool moveRightPressed = keyManager ? sf::Keyboard::isKeyPressed(keyManager->getKeyForAction(GameAction::MOVE_RIGHT))
                                       : sf::Keyboard::isKeyPressed(sf::Keyboard::Right);

    // Move left
    if (moveLeftPressed)
    {
        move(-moveSpeed * dt);
        moving = true;
        facingRight = false;
    }
    // Move right
    else if (moveRightPressed)
    {
        move(moveSpeed * dt);
        moving = true;
        facingRight = true;
    }

    // Switch animation state
    if (!isAttacking && moving != isRunning)
    {
        isRunning = moving;
        currentFrame = 0;
    }
}

// Apply gravity, platform, and ground collision logic
void Player::updatePhysics(float dt, float groundLevel, Platform **platforms, int numPlatforms)
{
    // Apply gravity when airborne
    if (!onGround)
    {
        velocity.y += gravity * dt;
    }

    position.y += velocity.y * dt;
    sprite.setPosition(position);

    float collisionBottom = position.y + collisionOffsetY + collisionHeight;
    float collisionLeft = position.x + collisionOffsetX;
    float collisionRight = position.x + collisionOffsetX + collisionWidth;

    bool landedOnSurface = false;

    // Ground collision check
    if (collisionBottom >= groundLevel && velocity.y >= 0)
    {
        float adjustedY = groundLevel - collisionOffsetY - collisionHeight;
        position.y = adjustedY;
        sprite.setPosition(position);
        velocity.y = 0.f;
        landedOnSurface = true;
        jumpCount = 0;
    }

    // Platform collision check
    for (int i = 0; i < numPlatforms; i++)
    {
        sf::FloatRect platformBounds = platforms[i]->getCollisionBounds();
        sf::Vector2f platformVelocity = platforms[i]->getVelocity();

        float tolerance = (platformVelocity.y > 0) ? 30.f : 20.f;

        if (velocity.y >= 0 &&
            collisionBottom >= platformBounds.top - 5.f &&
            collisionBottom <= platformBounds.top + tolerance &&
            collisionRight > platformBounds.left &&
            collisionLeft < platformBounds.left + platformBounds.width)
        {
            float adjustedY = platformBounds.top - collisionOffsetY - collisionHeight;
            position.y = adjustedY;

            position.y += platformVelocity.y * dt;

            sprite.setPosition(position);
            velocity.y = 0.f;
            landedOnSurface = true;
            jumpCount = 0;
            break;
        }
    }

    // Update ground state
    onGround = landedOnSurface;
}

// Return sprite bounding box
sf::FloatRect Player::getBounds() const
{
    return sprite.getGlobalBounds();
}

// Return collision bounding box (for hit detection)
sf::FloatRect Player::getCollisionBounds() const
{
    return sf::FloatRect({position.x + collisionOffsetX, position.y + collisionOffsetY},
                         {collisionWidth, collisionHeight});
}

// Return attack range area
sf::FloatRect Player::getAttackRange() const
{
    return sf::FloatRect({position.x + collisionOffsetX - 30.f, position.y + collisionOffsetY - 20.f},
                         {collisionWidth + 60.f, collisionHeight + 40.f});
}

// Get currently active animation
Animation *Player::getCurrentAnimation() const
{
    if (isAttacking || isSpecialAttacking)
    {
        if (attackType == 3)
            return attack2Anim;
        else
            return (attackType == 1) ? attack1Anim : attack2Anim;
    }
    else
        return isRunning ? runAnim : idleAnim;
}

// Begin attack animation
void Player::attack(int type)
{
    if (!isAttacking && !isSpecialAttacking)
    {
        isAttacking = true;
        attackType = type;
        currentFrame = 0;
        animationTimer = 0.f;
    }
}

// Begin special dash attack
void Player::specialAttack()
{
    if (!isAttacking && !isSpecialAttacking)
    {
        isSpecialAttacking = true;
        attackType = 3;
        currentFrame = 0;
        animationTimer = 0.f;
        specialDashTraveled = 0.f;
    }
}

// Handle jumping logic with multiple jumps
bool Player::jump(float dt, const KeyBindingManager *keyManager)
{
    bool currentSpaceState = keyManager ? sf::Keyboard::isKeyPressed(keyManager->getKeyForAction(GameAction::JUMP))
                                        : sf::Keyboard::isKeyPressed(sf::Keyboard::Space);

    bool didJump = false;

    // Only jump if not holding space and under max jumps
    if (currentSpaceState && !spacePressed && jumpCount < maxJumps)
    {
        velocity.y = jumpStrength;
        jumpCount++;
        if (onGround)
        {
            onGround = false;
        }
        didJump = true;
    }

    spacePressed = currentSpaceState;
    return didJump;
}

// Move horizontally by delta
void Player::move(float deltaX)
{
    position.x += deltaX;
    sprite.setPosition(position);
}

// Apply stat bonuses upon level-up
void Player::applyLevelUpBonuses(const LevelUpInfo &levelUpInfo)
{
    if (levelUpInfo.leveledUp)
    {
        int newMaxHp = getMaxHp() + levelUpInfo.hpGained;
        setMaxHp(newMaxHp);
        setHp(newMaxHp);

        int newDamage = getDamage() + levelUpInfo.damageGained;
        setDamage(newDamage);
    }
}

void Player::initializeProgressionStats()
{
    level = STARTING_LEVEL;
    experience = STARTING_EXPERIENCE;
    expToNextLevel = BASE_EXP_TO_NEXT_LEVEL;
}

Player::LevelUpInfo Player::gainExperience(int exp)
{
    LevelUpInfo info;
    experience += exp;

    while (experience >= expToNextLevel)
    {
        experience -= expToNextLevel;
        level++;
        expToNextLevel = level * EXP_MULTIPLIER_PER_LEVEL;

        info.hpGained += HP_GAIN_PER_LEVEL;
        info.damageGained += DAMAGE_GAIN_PER_LEVEL;
        info.leveledUp = true;
    }

    if (info.leveledUp)
    {
        info.newLevel = level;
    }

    return info;
}

int Player::getLevel() const { return level; }
int Player::getExperience() const { return experience; }
int Player::getExpToNextLevel() const { return expToNextLevel; }

void Player::setProgression(int newLevel, int newExperience, int newExpToNextLevel)
{
    level = (newLevel > 0) ? newLevel : STARTING_LEVEL;
    experience = (newExperience >= 0) ? newExperience : 0;
    if (newExpToNextLevel > 0)
    {
        expToNextLevel = newExpToNextLevel;
    }
    else
    {
        expToNextLevel = level * EXP_MULTIPLIER_PER_LEVEL;
    }
}

bool Player::addHpPotion()
{
    if (hpPotions < maxHpPotions)
    {
        hpPotions++;
        return true;
    }
    return false;
}

bool Player::useHpPotion()
{
    if (hpPotions > 0 && getHp() < getMaxHp())
    {
        hpPotions--;
        int healAmount = HPPotion::GetHealingValue();
        setHp(getHp() + healAmount);
        return true;
    }
    return false;
}

int Player::getHpPotions() const { return hpPotions; }
int Player::getMaxHpPotions() const { return maxHpPotions; }
void Player::setHpPotions(int count) { hpPotions = count; }

// Accessors
SkillTree &Player::getSkillTree() { return skillTree; }
const SkillTree &Player::getSkillTree() const { return skillTree; }
bool Player::isFacingRight() const { return facingRight; }
bool Player::getIsAttacking() const { return isAttacking; }
bool Player::getIsSpecialAttacking() const { return isSpecialAttacking; }
int Player::getAttackType() const { return attackType; }
float Player::getCollisionWidth() const { return collisionWidth; }
float Player::getCollisionHeight() const { return collisionHeight; }
float Player::getCollisionOffsetX() const { return collisionOffsetX; }
float Player::getCollisionOffsetY() const { return collisionOffsetY; }

// Mutators
void Player::setFacingRight(bool facing) { facingRight = facing; }
void Player::resetJumpCount() { jumpCount = 0; }
