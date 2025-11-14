#pragma once

#include "Projectile.h"

// Meteor class inherits from Projectile and represents a falling meteor projectile
class Meteor : public Projectile
{
private:
    int currentFrame;         // Current animation frame index
    float animationTimer;     // Timer to control animation frame changes
    sf::Texture *textures[5]; // Array of texture frames for meteor animation

public:
    // Constructor initializes meteor with textures and a start position
    Meteor(sf::Texture *meteorTextures[5], sf::Vector2f startPos);

    // Reset functions to reuse meteor object
    void reset() override;
    void reset(sf::Texture *meteorTextures[5], sf::Vector2f startPos);

    // Update meteor movement and animation
    void update(float dt) override;

    // Return the bounding box for collision detection
    sf::FloatRect getBounds() const override;
};
