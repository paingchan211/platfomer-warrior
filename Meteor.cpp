#include "Meteor.h"
#include <cstdlib>

// Constructor initializes meteor with textures and position
Meteor::Meteor(sf::Texture *meteorTextures[5], sf::Vector2f startPos)
    : Projectile()
{
    // Copy texture pointers
    for (int i = 0; i < 5; i++)
    {
        textures[i] = meteorTextures[i];
    }

    // Set initial texture and position
    sprite.setTexture(*textures[0]);
    position = startPos;
    sprite.setPosition(position);

    // Assign random fall speed
    float fallSpeed = 250.f + (rand() % 100);
    velocity = sf::Vector2f(0.f, fallSpeed);

    // Initialize state
    active = true;
    currentFrame = 0;
    animationTimer = 0.f;
    initializeProjectile(25, 10.0f); // Set damage and lifetime

    // Scale meteor sprite
    sprite.setScale({3.f, 3.f});
}

// Update meteor position, animation, and lifetime
void Meteor::update(float dt)
{
    if (!active)
        return;

    // Move meteor
    position += velocity * dt;
    sprite.setPosition(position);

    // Animate meteor frames
    animationTimer += dt;
    float frameTime = 0.1f;
    if (animationTimer >= frameTime)
    {
        animationTimer -= frameTime;
        currentFrame = (currentFrame + 1) % 5;
        sprite.setTexture(*textures[currentFrame]);
    }

    // Update lifetime
    updateLifetime(dt);
}

// Return meteor collision bounds with margin
sf::FloatRect Meteor::getBounds() const
{
    sf::FloatRect bounds = sprite.getGlobalBounds();

    float margin = 20.f;
    return sf::FloatRect(bounds.left + margin, bounds.top + margin,
                         bounds.width - (2 * margin), bounds.height - (2 * margin));
}

// Empty reset for base compatibility
void Meteor::reset() {}

// Reset meteor with new textures and position
void Meteor::reset(sf::Texture *meteorTextures[5], sf::Vector2f startPos)
{
    // Reload textures
    for (int i = 0; i < 5; i++)
    {
        textures[i] = meteorTextures[i];
    }

    // Reset appearance and position
    sprite.setTexture(*textures[0]);
    position = startPos;
    sprite.setPosition(position);

    // Randomize fall speed again
    float fallSpeed = 250.f + (rand() % 100);
    velocity = sf::Vector2f(0.f, fallSpeed);

    // Reset animation and state
    active = true;
    currentFrame = 0;
    animationTimer = 0.f;
    initializeProjectile(25, 10.0f);

    // Restore scale
    sprite.setScale({3.f, 3.f});
}
