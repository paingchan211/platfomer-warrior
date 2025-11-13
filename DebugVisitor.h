#pragma once

#include "EntityVisitor.h"
#include <SFML/Graphics.hpp>

// Debug visitor class that draws collision bounds for entities (visitor pattern implementation)
class DebugVisitor : public EntityVisitor
{
public:
    // Constructor that takes a reference to the render window
    explicit DebugVisitor(sf::RenderWindow &window);

    // Visits player entity and draws its collision bounds
    void visit(const Player &player) override;
    // Visits enemy entity and draws its collision bounds
    void visit(const Enemy &enemy) override;
    // Visits boss entity and draws its collision bounds
    void visit(const Boss &boss) override;
    // Visits platform entity and draws its collision bounds
    void visit(const Platform &platform) override;
    // Visits HP potion entity and draws its collision bounds
    void visit(const HPPotion &potion) override;
    // Visits fire projectile entity and draws its collision bounds
    void visit(const FireProjectile &projectile) override;
    // Visits ice projectile entity and draws its collision bounds
    void visit(const IceProjectile &projectile) override;
    // Visits meteor entity and draws its collision bounds
    void visit(const Meteor &meteor) override;

private:
    // Draws collision bounds as a colored outline rectangle
    void drawBounds(const sf::FloatRect &bounds, const sf::Color &color) const;

    sf::RenderWindow &window; // Reference to the render window for drawing
    static constexpr float OUTLINE_THICKNESS = 2.f; // Thickness of the debug outline in pixels
};
