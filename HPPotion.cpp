#include "HPPotion.h"
#include "EntityVisitor.h"

// Constructor initializing position, animation, and lifetime
HPPotion::HPPotion(const sf::Texture &texture, sf::Vector2f startPos)
{
    sprite.setTexture(texture);
    position = startPos;
    sprite.setPosition(position);
    active = true;
    currentFrame = 0;
    animationTimer = 0.f;
    healingValue = HEALING_VALUE;
    lifetime = 10.0f;

    // Set initial animation frame and scale
    sprite.setTextureRect(sf::IntRect({0, 0}, {FRAME_WIDTH, FRAME_HEIGHT}));
    sprite.setScale({2.0f, 2.0f});
}

// Accept visitor (part of the visitor pattern)
void HPPotion::accept(EntityVisitor &visitor) const
{
    visitor.visit(*this);
}

// Updates potion animation and lifetime
void HPPotion::update(float dt)
{
    if (!active)
        return;

    animationTimer += dt;
    float frameTime = 0.15f;

    // Animate sprite over time
    if (animationTimer >= frameTime)
    {
        animationTimer -= frameTime;
        currentFrame = (currentFrame + 1) % TOTAL_FRAMES;
        sprite.setTextureRect(sf::IntRect({currentFrame * FRAME_WIDTH, 0}, {FRAME_WIDTH, FRAME_HEIGHT}));
    }

    // Decrease lifetime and deactivate when expired
    lifetime -= dt;
    if (lifetime <= 0.f)
    {
        active = false;
    }
}

// Returns the potion’s collision bounds with margin applied
sf::FloatRect HPPotion::getBounds() const
{
    sf::FloatRect bounds = sprite.getGlobalBounds();
    float margin = 5.f;
    return sf::FloatRect(bounds.left + margin, bounds.top + margin,
                         bounds.width - (2 * margin), bounds.height - (2 * margin));
}

// Resets the potion to a new position and reinitializes animation and state
void HPPotion::reset(const sf::Texture &texture, sf::Vector2f startPos)
{
    sprite.setTexture(texture);
    position = startPos;
    sprite.setPosition(position);
    active = true;
    currentFrame = 0;
    animationTimer = 0.f;
    healingValue = HEALING_VALUE;
    lifetime = 10.0f;

    sprite.setTextureRect(sf::IntRect({0, 0}, {FRAME_WIDTH, FRAME_HEIGHT}));
    sprite.setScale({2.0f, 2.0f});
}

// Returns current healing amount
int HPPotion::getHealingValue() const
{
    return healingValue;
}

// Returns constant healing value (static)
int HPPotion::GetHealingValue()
{
    return HEALING_VALUE;
}