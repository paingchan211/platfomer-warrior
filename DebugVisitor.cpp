#include "DebugVisitor.h"
#include "Player.h"
#include "Enemy.h"
#include "Boss.h"
#include "Platform.h"
#include "HPPotion.h"
#include "FireProjectile.h"
#include "IceProjectile.h"
#include "Meteor.h"

// Constructor that takes a reference to the render window
DebugVisitor::DebugVisitor(sf::RenderWindow &window)
    : window(window) {}

// Visits player entity and draws its collision bounds in green
void DebugVisitor::visit(const Player &player)
{
    drawBounds(player.getCollisionBounds(), sf::Color::Green);
}

// Visits enemy entity and draws its collision bounds in red (only if alive)
void DebugVisitor::visit(const Enemy &enemy)
{
    // Skip drawing if enemy is dead
    if (!enemy.isAlive())
        return;

    drawBounds(enemy.getCollisionBounds(), sf::Color::Red);
}

// Visits boss entity and draws its collision bounds in magenta (only if alive)
void DebugVisitor::visit(const Boss &boss)
{
    // Skip drawing if boss is dead
    if (!boss.isAlive())
        return;

    drawBounds(boss.getCollisionBounds(), sf::Color::Magenta);
}

// Visits platform entity and draws its collision bounds in yellow
void DebugVisitor::visit(const Platform &platform)
{
    drawBounds(platform.getCollisionBounds(), sf::Color::Yellow);
}

// Visits HP potion entity and draws its collision bounds in green (only if active)
void DebugVisitor::visit(const HPPotion &potion)
{
    // Skip drawing if potion is not active
    if (!potion.isActive())
        return;

    drawBounds(potion.getBounds(), sf::Color(0, 200, 0));
}

// Visits fire projectile entity and draws its collision bounds in cyan (only if active)
void DebugVisitor::visit(const FireProjectile &projectile)
{
    // Skip drawing if projectile is not active
    if (!projectile.isActive())
        return;

    drawBounds(projectile.getBounds(), sf::Color::Cyan);
}

// Visits ice projectile entity and draws its collision bounds in blue (only if active)
void DebugVisitor::visit(const IceProjectile &projectile)
{
    // Skip drawing if projectile is not active
    if (!projectile.isActive())
        return;

    drawBounds(projectile.getBounds(), sf::Color::Blue);
}

// Visits meteor entity and draws its collision bounds in orange (only if active)
void DebugVisitor::visit(const Meteor &meteor)
{
    // Skip drawing if meteor is not active
    if (!meteor.isActive())
        return;

    drawBounds(meteor.getBounds(), sf::Color(255, 128, 0));
}

// Draws collision bounds as a colored outline rectangle
void DebugVisitor::drawBounds(const sf::FloatRect &bounds, const sf::Color &color) const
{
    // Skip drawing if bounds are invalid
    if (bounds.width <= 0.f || bounds.height <= 0.f)
        return;

    // Create rectangle shape with transparent fill and colored outline
    sf::RectangleShape rect;
    rect.setPosition(bounds.left, bounds.top);
    rect.setSize(sf::Vector2f(bounds.width, bounds.height));
    rect.setFillColor(sf::Color::Transparent);
    rect.setOutlineColor(color);
    rect.setOutlineThickness(OUTLINE_THICKNESS);
    // Draw the rectangle to the window
    window.draw(rect);
}
