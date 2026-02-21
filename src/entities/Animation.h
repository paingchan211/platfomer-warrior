#pragma once
#include <string>
#include <SFML/Graphics.hpp>

// Sets the sprite direction by flipping horizontally based on facingRight flag
void setSpriteDirection(sf::Sprite &sprite, bool facingRight, sf::Vector2u frameSize, float scaleX = 1.f, float scaleY = 1.f);

// Checks if two rectangles intersect with each other
bool checkRectIntersection(const sf::FloatRect &rectA, const sf::FloatRect &rectB);

// Animation structure for single-row sprite animations
struct Animation
{
    int frames;             // Number of frames in the animation
    sf::Texture texture;    // Texture containing all animation frames
    sf::Vector2u frameSize; // Size of each individual frame

    // Default constructor initializing frames and frameSize to zero
    Animation();

    // Loads animation texture from file and sets frame count
    bool load(const std::string &file, int frameCount);

    // Returns the rectangle for a specific frame at the given index
    sf::IntRect frameRect(int index) const;
};

// Animation structure for multi-row sprite animations (used for bosses)
struct BossAnimation
{
    int rows;               // Number of rows in the animation sprite sheet
    int framesPerRow;       // Number of frames in each row
    sf::Texture texture;    // Texture containing all animation frames
    sf::Vector2u frameSize; // Size of each individual frame

    // Default constructor initializing rows, framesPerRow, and frameSize to zero
    BossAnimation();

    // Loads boss animation texture from file and sets frame and row counts
    bool load(const std::string &file, int frameCount, int rowCount);

    // Returns the rectangle for a specific frame at the given index and row
    sf::IntRect frameRect(int frameIndex, int row) const;
};

// Animation structure for enemies with separate run, attack, and idle animations
struct EnemyAnimation
{
    int runFrames;             // Number of frames in the run animation
    int attackFrames;          // Number of frames in the attack animation
    int idleFrames;            // Number of frames in the idle animation
    sf::Texture runTexture;    // Texture for run animation
    sf::Texture attackTexture; // Texture for attack animation
    sf::Texture idleTexture;   // Texture for idle animation
    sf::Vector2u frameSize;    // Size of each individual frame

    // Default constructor initializing all frame counts and frameSize to zero
    EnemyAnimation();

    // Loads run animation texture from file and sets frame count
    bool loadRun(const std::string &file, int frameCount);

    // Loads attack animation texture from file and sets frame count
    bool loadAttack(const std::string &file, int frameCount);

    // Loads idle animation texture from file and sets frame count
    bool loadIdle(const std::string &file, int frameCount);

    // Returns the rectangle for a specific run animation frame at the given index
    sf::IntRect runFrameRect(int index) const;

    // Returns the rectangle for a specific attack animation frame at the given index
    sf::IntRect attackFrameRect(int index) const;

    // Returns the rectangle for a specific idle animation frame at the given index
    sf::IntRect idleFrameRect(int index) const;
};
