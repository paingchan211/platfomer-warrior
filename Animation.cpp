#include "Animation.h"

// Sets the sprite direction by flipping horizontally based on facingRight flag
void setSpriteDirection(sf::Sprite &sprite, bool facingRight, sf::Vector2u frameSize, float scaleX, float scaleY)
{
    // If facing left, flip the sprite horizontally and adjust origin
    if (!facingRight)
    {
        // Flip sprite by using negative scale and set origin to right edge
        sprite.setScale({-scaleX, scaleY});
        sprite.setOrigin({static_cast<float>(frameSize.x), 0.f});
    }
    // If facing right, use normal scale and origin at top-left
    else
    {
        // Set normal scale and origin at top-left corner
        sprite.setScale({scaleX, scaleY});
        sprite.setOrigin({0.f, 0.f});
    }
}

// Checks if two rectangles intersect with each other using AABB collision detection
bool checkRectIntersection(const sf::FloatRect &rectA, const sf::FloatRect &rectB)
{
    // Check if rectangles overlap on both X and Y axes
    return (rectA.left < rectB.left + rectB.width &&
            rectA.left + rectA.width > rectB.left &&
            rectA.top < rectB.top + rectB.height &&
            rectA.top + rectA.height > rectB.top);
}

// Default constructor initializing frames and frameSize to zero
Animation::Animation() : frames(0), frameSize(0, 0) {}

// Loads animation texture from file and sets frame count
bool Animation::load(const std::string &file, int frameCount)
{
    // Try to load texture from file, return false if it fails
    if (!texture.loadFromFile(file))
        return false;
    // Set frame count and calculate frame size based on texture dimensions
    frames = frameCount;
    auto size = texture.getSize();
    // Calculate frame size by dividing texture width by frame count
    frameSize = {size.x / frameCount, size.y};
    return true;
}

// Returns the rectangle for a specific frame at the given index
sf::IntRect Animation::frameRect(int index) const
{
    // Calculate frame rectangle based on index and frame size
    return {{int(index * frameSize.x), 0},
            {int(frameSize.x), int(frameSize.y)}};
}

// Default constructor initializing rows, framesPerRow, and frameSize to zero
BossAnimation::BossAnimation() : framesPerRow(0), rows(0), frameSize(0, 0) {}

// Loads boss animation texture from file and sets frame and row counts
bool BossAnimation::load(const std::string &file, int frameCount, int rowCount)
{
    // Try to load texture from file, return false if it fails
    if (!texture.loadFromFile(file))
        return false;
    // Set frame and row counts, then calculate frame size
    framesPerRow = frameCount;
    rows = rowCount;
    auto size = texture.getSize();
    // Calculate frame size by dividing texture dimensions by frame and row counts
    frameSize = {size.x / frameCount, size.y / rowCount};
    return true;
}

// Returns the rectangle for a specific frame at the given index and row
sf::IntRect BossAnimation::frameRect(int frameIndex, int row) const
{
    // Calculate frame rectangle based on frame index, row, and frame size
    return {{int(frameIndex * frameSize.x), int(row * frameSize.y)},
            {int(frameSize.x), int(frameSize.y)}};
}

// Default constructor initializing all frame counts and frameSize to zero
EnemyAnimation::EnemyAnimation() : runFrames(0), attackFrames(0), idleFrames(0), frameSize(0, 0) {}

// Loads run animation texture from file and sets frame count
bool EnemyAnimation::loadRun(const std::string &file, int frameCount)
{
    // Try to load run texture from file, return false if it fails
    if (!runTexture.loadFromFile(file))
        return false;
    // Set run frame count and calculate frame size
    runFrames = frameCount;
    auto size = runTexture.getSize();
    // Calculate frame size by dividing texture width by frame count
    frameSize = {size.x / frameCount, size.y};
    return true;
}

// Loads attack animation texture from file and sets frame count
bool EnemyAnimation::loadAttack(const std::string &file, int frameCount)
{
    // Try to load attack texture from file, return false if it fails
    if (!attackTexture.loadFromFile(file))
        return false;
    // Set attack frame count
    attackFrames = frameCount;

    // Calculate attack frame size based on texture dimensions
    auto size = attackTexture.getSize();
    sf::Vector2u attackFrameSize = {size.x / frameCount, size.y};

    // Update frame size to match attack animation frame size
    frameSize = attackFrameSize;
    return true;
}

// Loads idle animation texture from file and sets frame count
bool EnemyAnimation::loadIdle(const std::string &file, int frameCount)
{
    // Try to load idle texture from file, return false if it fails
    if (!idleTexture.loadFromFile(file))
        return false;
    // Set idle frame count
    idleFrames = frameCount;

    // Calculate idle frame size based on texture dimensions
    auto size = idleTexture.getSize();
    sf::Vector2u idleFrameSize = {size.x / frameCount, size.y};
    // Update frame size to match idle animation frame size
    frameSize = idleFrameSize;
    return true;
}

// Returns the rectangle for a specific run animation frame at the given index
sf::IntRect EnemyAnimation::runFrameRect(int index) const
{
    // Calculate run frame rectangle based on index and frame size
    return {{int(index * frameSize.x), 0},
            {int(frameSize.x), int(frameSize.y)}};
}

// Returns the rectangle for a specific attack animation frame at the given index
sf::IntRect EnemyAnimation::attackFrameRect(int index) const
{
    // Calculate attack frame rectangle based on index and frame size
    return {{int(index * frameSize.x), 0},
            {int(frameSize.x), int(frameSize.y)}};
}

// Returns the rectangle for a specific idle animation frame at the given index
sf::IntRect EnemyAnimation::idleFrameRect(int index) const
{
    // Calculate idle frame rectangle based on index and frame size
    return {{int(index * frameSize.x), 0},
            {int(frameSize.x), int(frameSize.y)}};
}
