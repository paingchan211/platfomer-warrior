#pragma once

#include "Projectile.h"
#include "Animation.h"

// Represents an ice projectile that slows and freezes enemies
class IceProjectile : public Projectile
{
private:
    // Animation constants
    static constexpr int FRAME_WIDTH = 48;
    static constexpr int FRAME_HEIGHT = 32;
    static constexpr int TOTAL_FRAMES = 10;

    float animTime;       // Animation timer
    int currentFrame;     // Current frame index
    float freezeDuration; // Duration target is frozen
    float slowAmount;     // Movement slow multiplier

public:
    IceProjectile(const sf::Texture &texture, const sf::Vector2f &startPos, bool facingRight); // Constructor

    void reset() override;                                                                  // Default reset
    void reset(const sf::Texture &texture, const sf::Vector2f &startPos, bool facingRight); // Full reset

    void update(float dt) override;                     // Updates position and animation
    void accept(EntityVisitor &visitor) const override; // Accepts visitor
    sf::FloatRect getBounds() const override;           // Returns collision bounds

    float getFreezeDuration() const; // Returns freeze duration
    float getSlowAmount() const;     // Returns slow effect
};