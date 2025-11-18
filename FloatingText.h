#pragma once

#include <SFML/Graphics.hpp>
#include <string>

// Class representing a floating text effect (e.g., damage or healing numbers)
class FloatingText
{
public:
    // Constructor that initializes floating text with font, value, position, color, and type (healing or damage)
    FloatingText(const sf::Font &font, int damage, sf::Vector2f position, const sf::Color &color = sf::Color::Red, bool isHealing = false);

    // Destructor
    ~FloatingText();

    // Updates the text position and fade animation over time
    void update(float dt);

    // Draws the floating text to the render window
    void draw(sf::RenderWindow &window) const;

    // Checks if the floating text has expired (fully faded out)
    bool isExpired() const;

    // Returns the text currently being displayed (e.g., damage/heal amount)
    std::string getDisplayString() const;

private:
    sf::Text text;         // SFML text object used for displaying damage/heal value
    sf::Vector2f velocity; // Movement speed and direction (e.g., floating upward)
    float lifetime;        // Remaining time before text disappears
    float maxLifetime;     // Total duration for which text remains visible
    sf::Color baseColor;   // Base color used for fading transparency
};
