#include "FireProjectile.h"
#include "EntityVisitor.h"

// Constructor that initializes the fire projectile with texture, position, and direction
FireProjectile::FireProjectile(const sf::Texture &texture, const sf::Vector2f &startPos, bool facingRight)
    : Projectile()
{
    // Set sprite texture and position
    sprite.setTexture(texture);
    position = startPos;
    sprite.setPosition(position);

    // Initialize animation state
    currentFrame = 0;
    animTime = 0.f;
    sprite.setTextureRect(sf::IntRect({0, 0}, {FRAME_WIDTH, FRAME_HEIGHT}));

    // Set sprite scale
    sprite.setScale({3.0f, 3.0f});

    // Set velocity based on facing direction
    float speed = 300.f;
    velocity = facingRight ? sf::Vector2f(speed, 0.f) : sf::Vector2f(-speed, 0.f);

    // Set sprite direction
    setSpriteDirection(sprite, facingRight, {FRAME_WIDTH, FRAME_HEIGHT}, 3.0f, 3.0f);

    // Initialize projectile as active
    active = true;
    initializeProjectile(20, 3.0f);

    // Initialize burn effect properties
    burnDamage = 5;
    burnDuration = 3.0f;
}

// Accepts a visitor for the visitor pattern
void FireProjectile::accept(EntityVisitor &visitor) const
{
    visitor.visit(*this);
}

// Resets the projectile to default state (empty implementation)
void FireProjectile::reset() {}

// Resets the projectile with new texture, position, and direction
void FireProjectile::reset(const sf::Texture &texture, const sf::Vector2f &startPos, bool facingRight)
{
    // Set sprite texture and position
    sprite.setTexture(texture);
    position = startPos;
    sprite.setPosition(position);

    // Reset animation state
    currentFrame = 0;
    animTime = 0.f;
    sprite.setTextureRect(sf::IntRect({0, 0}, {FRAME_WIDTH, FRAME_HEIGHT}));

    // Set sprite scale
    sprite.setScale({3.0f, 3.0f});

    // Set velocity based on facing direction
    float speed = 300.f;
    velocity = facingRight ? sf::Vector2f(speed, 0.f) : sf::Vector2f(-speed, 0.f);

    // Set sprite direction
    setSpriteDirection(sprite, facingRight, {FRAME_WIDTH, FRAME_HEIGHT}, 3.0f, 3.0f);

    // Reset projectile as active
    active = true;
    initializeProjectile(20, 3.0f);

    // Reset burn effect properties
    burnDamage = 5;
    burnDuration = 3.0f;
}

// Updates the projectile position, animation, and lifetime
void FireProjectile::update(float dt)
{
    // Return early if projectile is not active
    if (!active)
        return;

    // Update position based on velocity
    position += velocity * dt;
    sprite.setPosition(position);

    // Update animation
    animTime += dt;
    float frameTime = 0.1f;
    // Advance animation frame when frame time is reached
    if (animTime >= frameTime)
    {
        animTime -= frameTime;
        currentFrame = (currentFrame + 1) % TOTAL_FRAMES;
        sprite.setTextureRect(sf::IntRect({currentFrame * FRAME_WIDTH, 0}, {FRAME_WIDTH, FRAME_HEIGHT}));
    }

    // Update projectile lifetime
    updateLifetime(dt);
}

// Returns the collision bounds of the projectile (smaller than visual bounds)
sf::FloatRect FireProjectile::getBounds() const
{
    // Get sprite bounds and apply margin to create smaller collision box
    sf::FloatRect bounds = sprite.getGlobalBounds();
    float margin = 130.f;
    return sf::FloatRect(bounds.left + margin, bounds.top + margin,
                         bounds.width - (2 * margin), bounds.height - (2 * margin));
}

// Returns the burn damage applied by this projectile
int FireProjectile::getBurnDamage() const
{
    return burnDamage;
}

// Returns the burn duration applied by this projectile
float FireProjectile::getBurnDuration() const
{
    return burnDuration;
}
