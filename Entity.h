#pragma once
#include <SFML/Graphics.hpp>

// Base entity class that provides common functionality for all game entities
class Entity
{
protected:
    sf::Sprite sprite; // Sprite for rendering the entity
    sf::Vector2f position; // Position of the entity in the world
    sf::Vector2f velocity; // Velocity of the entity
    bool active; // Whether the entity is currently active

public:
    // Default constructor that initializes entity with default values
    Entity();
    // Virtual destructor for proper inheritance
    virtual ~Entity() = default;

    // Pure virtual update method that must be implemented by derived classes
    virtual void update(float dt) = 0;
    // Returns the global bounds of the entity sprite
    virtual sf::FloatRect getBounds() const;

    // Returns the position of the entity
    sf::Vector2f getPosition() const;
    // Returns the velocity of the entity
    sf::Vector2f getVelocity() const;
    // Returns whether the entity is active
    bool isActive() const;
    // Returns a const reference to the entity's sprite
    const sf::Sprite &getSprite() const;

    // Sets the position of the entity
    void setPosition(sf::Vector2f pos);
    // Sets the velocity of the entity
    void setVelocity(sf::Vector2f vel);
    // Sets whether the entity is active
    void setActive(bool isActive);
};
