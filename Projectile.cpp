#include "Projectile.h"

// Default constructor sets basic values
Projectile::Projectile()
    : Entity(), damage(10), lifetime(5.0f)
{
}

// Return projectile damage
int Projectile::getDamage() const
{
    return damage;
}

// Return projectile remaining lifetime
float Projectile::getLifetime() const
{
    return lifetime;
}

// Initialize damage and lifetime values
void Projectile::initializeProjectile(int initialDamage, float initialLifetime)
{
    damage = initialDamage;
    lifetime = initialLifetime;
}

// Decrease lifetime each frame; deactivate when expired
void Projectile::updateLifetime(float dt)
{
    lifetime -= dt;
    if (lifetime <= 0.f)
    {
        active = false;
    }
}