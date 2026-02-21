#include "CameraController.h"
#include <algorithm>
#include <iostream>

// Constructor that initializes the camera with screen dimensions and world width
CameraController::CameraController(float screenWidth, float screenHeight, float worldWidth)
    : screenWidth(screenWidth),
      screenHeight(screenHeight),
      worldWidth(worldWidth)
{
    // Initialize camera view with screen dimensions starting at origin
    camera = sf::View(sf::FloatRect(0.f, 0.f, screenWidth, screenHeight));
}

// Makes the camera follow the player's position, clamped to world bounds
void CameraController::followPlayer(const Player *player)
{
    // Return early if player is null
    if (!player)
    {
        return;
    }

    // Get player position
    sf::Vector2f playerPos = player->getPosition();

    // Clamp camera X position to keep it within world bounds
    float cameraX = std::clamp(playerPos.x,
                               screenWidth / 2.f,
                               worldWidth - screenWidth / 2.f);

    // Set camera center to follow player horizontally, centered vertically
    camera.setCenter(cameraX, screenHeight / 2.f);
}

// Sets the camera position directly (will be clamped to world bounds)
void CameraController::setPosition(const sf::Vector2f &position)
{
    // Clamp position to world bounds before setting
    sf::Vector2f clampedPos = clampToWorldBounds(position);
    camera.setCenter(clampedPos);
}

// Applies the camera view to the render window
void CameraController::applyToWindow(sf::RenderWindow &window) const
{
    window.setView(camera);
}

// Handles window resize events by updating camera dimensions
void CameraController::handleResize(float newWidth, float newHeight)
{
    // Update screen dimensions
    screenWidth = newWidth;
    screenHeight = newHeight;

    // Recreate camera view with new dimensions
    camera = sf::View(sf::FloatRect(0.f, 0.f, newWidth, newHeight));
}

// Clamps a position to stay within the world bounds
sf::Vector2f CameraController::clampToWorldBounds(const sf::Vector2f &position) const
{
    sf::Vector2f clamped = position;

    // Clamp X position to prevent camera from going outside world bounds
    clamped.x = std::clamp(position.x,
                           screenWidth / 2.f,
                           worldWidth - screenWidth / 2.f);

    // Clamp Y position to center of screen (camera stays at fixed Y)
    clamped.y = std::clamp(position.y,
                           screenHeight / 2.f,
                           screenHeight / 2.f);

    return clamped;
}
