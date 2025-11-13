#include "Character.h"

// Default constructor that initializes character with default values
Character::Character()
    : Entity(), hp(100), maxHp(100), damage(10), onGround(false), speed(100.f)
{
}

// Applies damage to the character
void Character::takeDamage(int dmg)
{
    // Subtract damage from health
    hp -= dmg;
    // Ensure health doesn't go below zero
    if (hp < 0)
        hp = 0;
}

// Checks if the character is still alive (HP > 0)
bool Character::isAlive() const
{
    return hp > 0;
}

// Returns the current health points
int Character::getHp() const
{
    return hp;
}

// Returns the maximum health points
int Character::getMaxHp() const
{
    return maxHp;
}

// Returns the damage value
int Character::getDamage() const
{
    return damage;
}

// Returns whether the character is on the ground
bool Character::getOnGround() const
{
    return onGround;
}

// Returns the movement speed
float Character::getSpeed() const
{
    return speed;
}

// Sets the current health points (clamped to valid range)
void Character::setHp(int health)
{
    hp = health;
    // Clamp health to not exceed maximum HP
    if (hp > maxHp)
        hp = maxHp;
    // Clamp health to not go below zero
    if (hp < 0)
        hp = 0;
}

// Sets the maximum health points (must be non-negative)
void Character::setMaxHp(int maxHealth)
{
    maxHp = maxHealth;
    // Ensure maximum HP is non-negative
    if (maxHp < 0)
        maxHp = 0;
}

// Sets the damage value (must be non-negative)
void Character::setDamage(int dmg)
{
    damage = dmg;
    // Ensure damage is non-negative
    if (damage < 0)
        damage = 0;
}

// Sets whether the character is on the ground
void Character::setOnGround(bool ground)
{
    onGround = ground;
}

// Sets the movement speed (must be non-negative)
void Character::setSpeed(float spd)
{
    speed = spd;
    // Ensure speed is non-negative
    if (speed < 0.f)
        speed = 0.f;
}

// Initializes combat stats (HP and damage) for the character
void Character::initializeCombatStats(int initialHp, int initialDamage)
{
    // Set maximum HP and current HP to initial value
    maxHp = initialHp;
    hp = initialHp;
    // Set damage to initial damage value
    damage = initialDamage;
}
