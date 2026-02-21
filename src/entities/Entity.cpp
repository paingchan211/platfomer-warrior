#include "Entity.h"
#include "Constants.h"
#include <iostream>

// Default constructor that initializes entity with default values
Entity::Entity() : position(0.f, 0.f), velocity(0.f, 0.f), active(true)
{
    if (ENABLE_INHERITANCE_STDOUT)
    {
        std::cout << "[Inheritance] Entity::Entity() base constructed at position "
                  << "(" << position.x << ", " << position.y << ")" << std::endl;
    }
}

// Returns the global bounds of the entity sprite
sf::FloatRect Entity::getBounds() const
{
    return sprite.getGlobalBounds();
}

// Sets the position of the entity
void Entity::setPosition(sf::Vector2f pos)
{
    position = pos;
    sprite.setPosition(position);
}

// Returns the position of the entity
sf::Vector2f Entity::getPosition() const { return position; }
// Returns the velocity of the entity
sf::Vector2f Entity::getVelocity() const { return velocity; }
// Returns whether the entity is active
bool Entity::isActive() const { return active; }
// Returns a const reference to the entity's sprite
const sf::Sprite &Entity::getSprite() const { return sprite; }

// Sets the velocity of the entity
void Entity::setVelocity(sf::Vector2f vel) { velocity = vel; }
// Sets whether the entity is active
void Entity::setActive(bool isActive) { active = isActive; }
