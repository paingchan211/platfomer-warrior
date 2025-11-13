#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include "Player.h"
#include "Enemy.h"
#include "Boss.h"
#include "Platform.h"

// PhysicsManager handles gravity and collision detection for all entities
class PhysicsManager
{
public:
    PhysicsManager();  // Constructor
    ~PhysicsManager(); // Destructor

    // Update player physics with gravity and collision
    void updatePlayerPhysics(Player &player, float dt, float groundLevel,
                             Platform **platforms, int platformCount);

    // Update enemy physics with collision checks
    void updateEnemyPhysics(Enemy &enemy, float dt, float groundLevel,
                            Platform **platforms, int platformCount);

    // Update boss physics with platform/ground interaction
    void updateBossPhysics(Boss &boss, float dt, float groundLevel,
                           Platform **platforms, int platformCount);

    // Check if entity collides with the ground
    bool checkGroundCollision(const sf::FloatRect &entityBounds, float entityVelocityY,
                              float groundLevel, sf::Vector2f &outPosition, float currentY);

    // Check if entity collides with any platform
    bool checkPlatformCollision(const sf::FloatRect &entityBounds, float entityVelocityY,
                                Platform **platforms, int platformCount,
                                sf::Vector2f &outPosition, float currentY, float dt);

    // Set and get gravity value
    void setGravity(float gravity) { this->gravity = gravity; }
    float getGravity() const { return gravity; }

private:
    float gravity; // Gravity strength applied to all entities

    // Adjust entity position when hitting the ground
    void applyGroundCollision(sf::FloatRect bounds, sf::Vector2f currentPos,
                              float groundLevel, sf::Vector2f &outPosition);

    // Adjust entity position when landing on a platform
    bool applyPlatformCollision(sf::FloatRect bounds, sf::Vector2f currentPos,
                                Platform &platform, float velocityY, float dt,
                                sf::Vector2f &outPosition);
};