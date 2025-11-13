#pragma once

#include <SFML/Graphics.hpp>
#include "Player.h"

// Camera controller class that manages the camera view and follows the player
class CameraController
{
public:

    // Constructor that initializes the camera with screen dimensions and world width
    CameraController(float screenWidth, float screenHeight, float worldWidth);

    // Makes the camera follow the player's position, clamped to world bounds
    void followPlayer(const Player *player);

    // Sets the camera position directly (will be clamped to world bounds)
    void setPosition(const sf::Vector2f &position);

    // Returns the current camera view
    const sf::View &getView() const { return camera; }

    // Applies the camera view to the render window
    void applyToWindow(sf::RenderWindow &window) const;

    // Returns the center position of the camera
    sf::Vector2f getCenter() const { return camera.getCenter(); }

    // Sets the world width for boundary clamping
    void setWorldWidth(float worldWidth) { this->worldWidth = worldWidth; }

    // Handles window resize events by updating camera dimensions
    void handleResize(float newWidth, float newHeight);

private:
    sf::View camera; // SFML view object representing the camera
    float screenWidth; // Width of the screen/viewport
    float screenHeight; // Height of the screen/viewport
    float worldWidth; // Total width of the game world

    // Clamps a position to stay within the world bounds
    sf::Vector2f clampToWorldBounds(const sf::Vector2f &position) const;
};
