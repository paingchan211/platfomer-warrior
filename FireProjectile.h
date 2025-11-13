#pragma once

#include "Projectile.h"
#include "Animation.h"

// Fire projectile class that extends Projectile with burn effect capability
class FireProjectile : public Projectile
{
private:
    static constexpr int FRAME_WIDTH = 100; // Width of each animation frame in pixels
    static constexpr int FRAME_HEIGHT = 100; // Height of each animation frame in pixels
    static constexpr int TOTAL_FRAMES = 7; // Total number of animation frames

    float animTime; // Timer for animation frame updates
    int currentFrame; // Current animation frame index
    int burnDamage; // Damage per burn tick applied to enemies hit by this projectile
    float burnDuration; // Duration of burn effect in seconds

public:
    // Constructor that initializes the fire projectile with texture, position, and direction
    FireProjectile(const sf::Texture &texture, const sf::Vector2f &startPos, bool facingRight);

    // Resets the projectile to default state (empty implementation)
    void reset() override;
    // Resets the projectile with new texture, position, and direction
    void reset(const sf::Texture &texture, const sf::Vector2f &startPos, bool facingRight);

    // Updates the projectile position, animation, and lifetime
    void update(float dt) override;
    // Accepts a visitor for the visitor pattern
    void accept(EntityVisitor &visitor) const override;
    // Returns the collision bounds of the projectile (smaller than visual bounds)
    sf::FloatRect getBounds() const override;

    // Returns the burn damage applied by this projectile
    int getBurnDamage() const;
    // Returns the burn duration applied by this projectile
    float getBurnDuration() const;
};
