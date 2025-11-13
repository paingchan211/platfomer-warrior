#include "PhysicsManager.h"
#include "Constants.h"
#include <algorithm>

// Initialize physics manager with global gravity constant
PhysicsManager::PhysicsManager()
    : gravity(GRAVITY)
{
}

PhysicsManager::~PhysicsManager()
{
}

// Update player physics by delegating to player's own update method
void PhysicsManager::updatePlayerPhysics(Player &player, float dt, float groundLevel,
                                         Platform **platforms, int platformCount)
{
    player.updatePhysics(dt, groundLevel, platforms, platformCount);
}

// Update enemy physics and handle collisions
void PhysicsManager::updateEnemyPhysics(Enemy &enemy, float dt, float groundLevel,
                                        Platform **platforms, int platformCount)
{
    if (!enemy.isAlive())
        return;

    bool landed = false;
    sf::FloatRect enemyBounds = enemy.getCollisionBounds();
    sf::Vector2f enemyPos = enemy.getPosition();
    const float enemyBottom = enemyBounds.top + enemyBounds.height;

    // Ground collision check
    if (enemyBottom >= groundLevel && enemy.getVelocityY() >= 0.f)
    {
        const float offset = enemyBounds.top - enemyPos.y;
        enemyPos.y = groundLevel - enemyBounds.height - offset;
        enemy.setPosition(enemyPos);
        enemy.setVelocityY(0.f);
        landed = true;
    }
    else
    {
        // Platform collision check
        for (int pi = 0; pi < platformCount; ++pi)
        {
            if (!platforms[pi])
                continue;

            sf::FloatRect platformBounds = platforms[pi]->getCollisionBounds();
            sf::Vector2f platformVelocity = platforms[pi]->getVelocity();

            float tolerance = (platformVelocity.y > 0) ? 30.f : 20.f;

            if (enemy.getVelocityY() >= 0.f &&
                enemyBottom >= platformBounds.top - 5.f &&
                enemyBottom <= platformBounds.top + tolerance &&
                enemyBounds.left + enemyBounds.width > platformBounds.left &&
                enemyBounds.left < platformBounds.left + platformBounds.width)
            {
                const float offset = enemyBounds.top - enemyPos.y;
                enemyPos.y = platformBounds.top - enemyBounds.height - offset;
                enemyPos.y += platformVelocity.y * dt;
                enemy.setPosition(enemyPos);
                enemy.setVelocityY(0.f);
                landed = true;
                break;
            }
        }
    }

    enemy.setOnGround(landed);
}

// Update boss physics (similar to enemy but with vector velocity)
void PhysicsManager::updateBossPhysics(Boss &boss, float dt, float groundLevel,
                                       Platform **platforms, int platformCount)
{
    if (!boss.isAlive())
        return;

    bool bossLanded = false;
    sf::FloatRect bossBounds = boss.getCollisionBounds();
    sf::Vector2f bossPos = boss.getPosition();
    const float bossBottom = bossBounds.top + bossBounds.height;

    // Ground collision check
    if (bossBottom >= groundLevel && boss.getVelocity().y >= 0.f)
    {
        const float offset = bossBounds.top - bossPos.y;
        bossPos.y = groundLevel - bossBounds.height - offset;
        boss.setPosition(bossPos);
        sf::Vector2f velocity = boss.getVelocity();
        velocity.y = 0.f;
        boss.setVelocity(velocity);
        bossLanded = true;
    }
    else
    {
        // Platform collision check
        for (int pi = 0; pi < platformCount; ++pi)
        {
            if (!platforms[pi])
                continue;

            sf::FloatRect platformBounds = platforms[pi]->getCollisionBounds();
            sf::Vector2f platformVelocity = platforms[pi]->getVelocity();

            float tolerance = (platformVelocity.y > 0) ? 30.f : 20.f;

            if (boss.getVelocity().y >= 0.f &&
                bossBottom >= platformBounds.top - 5.f &&
                bossBottom <= platformBounds.top + tolerance &&
                bossBounds.left + bossBounds.width > platformBounds.left &&
                bossBounds.left < platformBounds.left + platformBounds.width)
            {
                const float offset = bossBounds.top - bossPos.y;
                bossPos.y = platformBounds.top - bossBounds.height - offset;
                bossPos.y += platformVelocity.y * dt;
                boss.setPosition(bossPos);
                sf::Vector2f velocity = boss.getVelocity();
                velocity.y = 0.f;
                boss.setVelocity(velocity);
                bossLanded = true;
                break;
            }
        }
    }

    boss.setOnGround(bossLanded);
}

// Check if entity touches the ground
bool PhysicsManager::checkGroundCollision(const sf::FloatRect &entityBounds, float entityVelocityY,
                                          float groundLevel, sf::Vector2f &outPosition, float currentY)
{
    const float entityBottom = entityBounds.top + entityBounds.height;

    if (entityBottom >= groundLevel && entityVelocityY >= 0.f)
    {
        const float offset = entityBounds.top - currentY;
        outPosition.y = groundLevel - entityBounds.height - offset;
        return true;
    }

    return false;
}

// Check if entity lands on any platform
bool PhysicsManager::checkPlatformCollision(const sf::FloatRect &entityBounds, float entityVelocityY,
                                            Platform **platforms, int platformCount,
                                            sf::Vector2f &outPosition, float currentY, float dt)
{
    const float entityBottom = entityBounds.top + entityBounds.height;

    for (int i = 0; i < platformCount; ++i)
    {
        if (!platforms[i])
            continue;

        sf::FloatRect platformBounds = platforms[i]->getCollisionBounds();
        sf::Vector2f platformVelocity = platforms[i]->getVelocity();

        float tolerance = (platformVelocity.y > 0) ? 30.f : 20.f;

        if (entityVelocityY >= 0.f &&
            entityBottom >= platformBounds.top - 5.f &&
            entityBottom <= platformBounds.top + tolerance &&
            entityBounds.left + entityBounds.width > platformBounds.left &&
            entityBounds.left < platformBounds.left + platformBounds.width)
        {
            const float offset = entityBounds.top - currentY;
            outPosition.y = platformBounds.top - entityBounds.height - offset;
            outPosition.y += platformVelocity.y * dt;
            return true;
        }
    }

    return false;
}

// Adjust entity position when it collides with ground
void PhysicsManager::applyGroundCollision(sf::FloatRect bounds, sf::Vector2f currentPos,
                                          float groundLevel, sf::Vector2f &outPosition)
{
    const float offset = bounds.top - currentPos.y;
    outPosition.y = groundLevel - bounds.height - offset;
}

// Adjust entity position when colliding with platform
bool PhysicsManager::applyPlatformCollision(sf::FloatRect bounds, sf::Vector2f currentPos,
                                            Platform &platform, float velocityY, float dt,
                                            sf::Vector2f &outPosition)
{
    sf::FloatRect platformBounds = platform.getCollisionBounds();
    sf::Vector2f platformVelocity = platform.getVelocity();

    const float entityBottom = bounds.top + bounds.height;
    float tolerance = (platformVelocity.y > 0) ? 30.f : 20.f;

    if (velocityY >= 0.f &&
        entityBottom >= platformBounds.top - 5.f &&
        entityBottom <= platformBounds.top + tolerance &&
        bounds.left + bounds.width > platformBounds.left &&
        bounds.left < platformBounds.left + platformBounds.width)
    {
        const float offset = bounds.top - currentPos.y;
        outPosition.y = platformBounds.top - bounds.height - offset;
        outPosition.y += platformVelocity.y * dt;
        return true;
    }

    return false;
}