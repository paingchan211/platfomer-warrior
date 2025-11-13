#include "Platform.h"
#include "EntityVisitor.h"

// Constructor for static platform
Platform::Platform(const sf::Texture &texture, float x, float y)
    : isMoving(false), moveSpeed(0.f), minY(y), maxY(y), movingUp(false)
{
    sprite.setTexture(texture);
    position = {x, y};
    sprite.setPosition(position);
    size = {static_cast<float>(texture.getSize().x), static_cast<float>(texture.getSize().y)};

    collisionSize = {size.x - (2 * PLATFORM_MARGIN_WIDTH), size.y - (2 * PLATFORM_MARGIN_HEIGHT)};
    collisionOffset = {PLATFORM_MARGIN_WIDTH, PLATFORM_MARGIN_HEIGHT};
    active = true;
    velocity = {0.f, 0.f};
}

// Constructor for moving platform
Platform::Platform(const sf::Texture &texture, float x, float y, float moveDistance, float speed)
    : isMoving(true), moveSpeed(speed), minY(y - moveDistance / 2.f), maxY(y + moveDistance / 2.f), movingUp(false)
{
    sprite.setTexture(texture);
    position = {x, y};
    sprite.setPosition(position);
    size = {static_cast<float>(texture.getSize().x), static_cast<float>(texture.getSize().y)};

    collisionSize = {size.x - (2 * PLATFORM_MARGIN_WIDTH), size.y - (2 * PLATFORM_MARGIN_HEIGHT)};
    collisionOffset = {PLATFORM_MARGIN_WIDTH, PLATFORM_MARGIN_HEIGHT};
    active = true;
    velocity = {0.f, 0.f};
}

// Visitor pattern entry point
void Platform::accept(EntityVisitor &visitor) const
{
    visitor.visit(*this);
}

// Update position if platform is moving
void Platform::update(float dt)
{
    if (!isMoving)
        return;

    if (movingUp)
    {
        velocity.y = -moveSpeed;
        position.y += velocity.y * dt;

        if (position.y <= minY)
        {
            position.y = minY;
            movingUp = false;
        }
    }
    else
    {
        velocity.y = moveSpeed;
        position.y += velocity.y * dt;

        if (position.y >= maxY)
        {
            position.y = maxY;
            movingUp = true;
        }
    }

    sprite.setPosition(position);
}

// Get sprite's global bounding box
sf::FloatRect Platform::getBounds() const
{
    return sprite.getGlobalBounds();
}

// Get reduced collision box
sf::FloatRect Platform::getCollisionBounds() const
{
    return sf::FloatRect({position.x + collisionOffset.x, position.y + collisionOffset.y}, collisionSize);
}