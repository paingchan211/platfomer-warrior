#include "IceProjectile.h"
#include "Constants.h"
#include <iostream>

// Constructor initializing projectile visuals, movement, and freezing effect
IceProjectile::IceProjectile(const sf::Texture &texture, const sf::Vector2f &startPos, bool facingRight)
    : Projectile()
{
    sprite.setTexture(texture);
    position = startPos;
    sprite.setPosition(position);

    currentFrame = 0;
    animTime = 0.f;
    sprite.setTextureRect(sf::IntRect({0, 0}, {FRAME_WIDTH, FRAME_HEIGHT}));
    sprite.setScale({2.5f, 2.5f});

    // Set direction and velocity based on facing side
    float speed = 350.f;
    velocity = facingRight ? sf::Vector2f(speed, 0.f) : sf::Vector2f(-speed, 0.f);
    setSpriteDirection(sprite, facingRight, {FRAME_WIDTH, FRAME_HEIGHT}, 2.5f, 2.5f);

    // Initialize projectile properties
    active = true;
    freezeDuration = 2.0f;
    slowAmount = 0.5f;
    initializeProjectile(8, 3.0f);

    if (ENABLE_INHERITANCE_STDOUT)
    {
        std::cout << "[Inheritance] IceProjectile constructed -> Projectile -> Entity | "
                  << "freezeDuration=" << freezeDuration
                  << ", slowAmount=" << slowAmount
                  << ", facingRight=" << (facingRight ? "true" : "false") << std::endl;
    }
}

// Resets projectile (no behavior defined)
void IceProjectile::reset() {}

// Resets projectile with new texture, position, and direction
void IceProjectile::reset(const sf::Texture &texture, const sf::Vector2f &startPos, bool facingRight)
{
    sprite.setTexture(texture);
    position = startPos;
    sprite.setPosition(position);

    currentFrame = 0;
    animTime = 0.f;
    sprite.setTextureRect(sf::IntRect({0, 0}, {FRAME_WIDTH, FRAME_HEIGHT}));
    sprite.setScale({2.5f, 2.5f});

    // Reapply direction and velocity
    float speed = 350.f;
    velocity = facingRight ? sf::Vector2f(speed, 0.f) : sf::Vector2f(-speed, 0.f);
    setSpriteDirection(sprite, facingRight, {FRAME_WIDTH, FRAME_HEIGHT}, 2.5f, 2.5f);

    // Reinitialize properties
    active = true;
    freezeDuration = 2.0f;
    slowAmount = 0.5f;
    initializeProjectile(8, 3.0f);
}

// Updates animation, movement, and lifetime
void IceProjectile::update(float dt)
{
    if (!active)
        return;

    // Move projectile
    position += velocity * dt;
    sprite.setPosition(position);

    // Animate frames
    animTime += dt;
    float frameTime = 0.08f;
    if (animTime >= frameTime)
    {
        animTime -= frameTime;
        currentFrame = (currentFrame + 1) % TOTAL_FRAMES;
        sprite.setTextureRect(sf::IntRect({currentFrame * FRAME_WIDTH, 0}, {FRAME_WIDTH, FRAME_HEIGHT}));
    }

    // Update lifetime and deactivate when expired
    updateLifetime(dt);
}

// Returns bounding box with margin for more accurate collisions
sf::FloatRect IceProjectile::getBounds() const
{
    sf::FloatRect bounds = sprite.getGlobalBounds();
    float margin = 30.f;
    return sf::FloatRect(bounds.left + margin, bounds.top + margin,
                         bounds.width - (2 * margin), bounds.height - (2 * margin));
}

// Returns duration that target stays frozen
float IceProjectile::getFreezeDuration() const
{
    return freezeDuration;
}

// Returns how much target is slowed
float IceProjectile::getSlowAmount() const
{
    return slowAmount;
}
