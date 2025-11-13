#pragma once

#include "Entity.h"

// Represents a health potion entity that heals the player
class HPPotion : public Entity
{
private:
    // Animation and healing constants
    static constexpr int FRAME_WIDTH = 18;
    static constexpr int FRAME_HEIGHT = 28;
    static constexpr int TOTAL_FRAMES = 13;
    static constexpr int HEALING_VALUE = 30;

    int currentFrame;     // Current animation frame
    float animationTimer; // Timer to control animation speed
    int healingValue;     // Amount of HP restored
    float lifetime;       // Duration before disappearing

public:
    HPPotion(const sf::Texture &texture, sf::Vector2f startPos); // Constructor

    void update(float dt) override;                     // Updates potion state
    void accept(EntityVisitor &visitor) const override; // Accepts entity visitor
    sf::FloatRect getBounds() const override;           // Returns collision bounds

    int getHealingValue() const;  // Returns heal amount
    static int GetHealingValue(); // Returns static heal amount

    void reset(const sf::Texture &texture, sf::Vector2f startPos); // Reinitializes potion
};