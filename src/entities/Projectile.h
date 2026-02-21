#pragma once
#include "Entity.h"

// Base projectile class (fireball, meteor, etc.)
class Projectile : public Entity
{
protected:
    int damage;     // Amount of damage dealt
    float lifetime; // Time until projectile deactivates

public:
    Projectile();
    virtual ~Projectile() = default;

    // Reset projectile (implemented by derived classes)
    virtual void reset() = 0;

    // Accessors
    int getDamage() const;
    float getLifetime() const;

protected:
    // Initialize projectile attributes
    void initializeProjectile(int initialDamage, float initialLifetime);

    // Reduce lifetime over time
    void updateLifetime(float dt);
};