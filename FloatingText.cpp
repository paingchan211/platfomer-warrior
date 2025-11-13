#include "FloatingText.h"

// Constructor that initializes floating text with given parameters
FloatingText::FloatingText(const sf::Font &font, int damage, sf::Vector2f position, const sf::Color &color, bool isHealing)
    : velocity(0.f, -50.f), // Upward movement speed
      lifetime(3.0f),       // Duration before fading out
      maxLifetime(3.0f),    // Maximum lifetime for alpha calculation
      baseColor(color)      // Store the base color for fading effect
{
    // Set the font for the text
    text.setFont(font);

    // Display "+" for healing, normal number for damage
    if (isHealing)
    {
        text.setString("+" + std::to_string(damage));
    }
    else
    {
        text.setString(std::to_string(damage));
    }

    // Configure text appearance
    text.setCharacterSize(24);
    text.setFillColor(color);
    text.setOutlineColor(sf::Color::Black);
    text.setOutlineThickness(2.f);

    // Center the origin of the text for proper alignment
    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(bounds.width / 2.f, bounds.height / 2.f);

    // Set initial position
    text.setPosition(position);
}

// Destructor (currently unused)
FloatingText::~FloatingText()
{
}

// Updates the floating text position and fade effect
void FloatingText::update(float dt)
{
    // Skip update if text has expired
    if (lifetime <= 0.f)
        return;

    // Move text upward based on velocity
    text.move(velocity * dt);

    // Reduce lifetime
    lifetime -= dt;

    // Calculate transparency based on remaining lifetime
    float alpha = (lifetime / maxLifetime) * 255.f;

    // Update text color with new alpha
    sf::Color currentColor = baseColor;
    currentColor.a = static_cast<sf::Uint8>(alpha);
    text.setFillColor(currentColor);

    // Update outline color with slightly lower opacity
    sf::Color outlineColor = sf::Color::Black;
    outlineColor.a = static_cast<sf::Uint8>(alpha * 0.8f);
    text.setOutlineColor(outlineColor);
}

// Draws the floating text on the window if not expired
void FloatingText::draw(sf::RenderWindow &window) const
{
    if (lifetime > 0.f)
    {
        window.draw(text);
    }
}

// Returns true if the floating text has faded out completely
bool FloatingText::isExpired() const
{
    return lifetime <= 0.f;
}
