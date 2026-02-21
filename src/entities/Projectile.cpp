#include "Projectile.h"
#include "Constants.h"
#include <iostream>

// Default constructor sets basic values
Projectile::Projectile()
    : Entity(), damage(10), lifetime(5.0f)
{
    if (ENABLE_INHERITANCE_STDOUT)
    {
        std::cout << "[Inheritance] Projectile::Projectile() extends Entity | "
                  << "damage=" << damage
                  << ", lifetime=" << lifetime << std::endl;
    }
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
