#pragma once
#include "Entity.h"

// Base character class that provides combat stats and common character functionality
class Character : public Entity
{
protected:
    int hp; // Current health points
    int maxHp; // Maximum health points
    int damage; // Damage dealt by this character
    bool onGround; // Whether the character is currently on the ground
    float speed; // Movement speed of the character

public:
    // Default constructor that initializes character with default values
    Character();
    // Virtual destructor for proper inheritance
    virtual ~Character() = default;

    // Applies damage to the character
    virtual void takeDamage(int dmg);
    // Checks if the character is still alive (HP > 0)
    virtual bool isAlive() const;

    // Returns the current health points
    int getHp() const;
    // Returns the maximum health points
    int getMaxHp() const;
    // Returns the damage value
    int getDamage() const;
    // Returns whether the character is on the ground
    bool getOnGround() const;
    // Returns the movement speed
    float getSpeed() const;

    // Sets the current health points (clamped to valid range)
    void setHp(int health);
    // Sets the maximum health points (must be non-negative)
    void setMaxHp(int maxHealth);
    // Sets the damage value (must be non-negative)
    void setDamage(int dmg);
    // Sets whether the character is on the ground
    void setOnGround(bool ground);
    // Sets the movement speed (must be non-negative)
    void setSpeed(float spd);

protected:
    // Initializes combat stats (HP and damage) for the character
    void initializeCombatStats(int initialHp, int initialDamage);
};
