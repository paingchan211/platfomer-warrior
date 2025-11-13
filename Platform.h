#pragma once

#include "Entity.h"

// Constants for platform collision margin
const float PLATFORM_MARGIN_WIDTH = 40.f;
const float PLATFORM_MARGIN_HEIGHT = 20.f;

// Platform class represents static or moving ground surfaces
class Platform : public Entity
{
private:
    sf::Vector2f size;            // Platform visual size
    sf::Vector2f collisionSize;   // Collision box size (smaller than sprite)
    sf::Vector2f collisionOffset; // Offset from sprite to collision box

    bool isMoving;   // Whether the platform moves vertically
    float moveSpeed; // Speed of vertical movement
    float minY;      // Minimum Y boundary
    float maxY;      // Maximum Y boundary
    bool movingUp;   // Current movement direction flag

public:
    // Static platform constructor
    Platform(const sf::Texture &texture, float x, float y);

    // Moving platform constructor
    Platform(const sf::Texture &texture, float x, float y, float moveDistance, float speed);

    // Update position (for moving platforms)
    void update(float dt) override;

    // Accept visitor for rendering or debugging
    void accept(EntityVisitor &visitor) const override;

    // Get platform sprite bounds
    sf::FloatRect getBounds() const override;

    // Get reduced collision bounds
    sf::FloatRect getCollisionBounds() const;

    // Get platform's current velocity
    sf::Vector2f getVelocity() const { return velocity; }
};