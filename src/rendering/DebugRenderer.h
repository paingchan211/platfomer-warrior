#pragma once

#include <SFML/Graphics.hpp>

class Player;
class Enemy;
class Boss;
class Platform;
class HPPotion;
class FireProjectile;
class IceProjectile;
class Meteor;

// Utility that draws collision bounds for various entity types
class DebugRenderer
{
public:
    explicit DebugRenderer(sf::RenderWindow &window);

    void draw(const Player &player);
    void draw(const Enemy &enemy);
    void draw(const Boss &boss);
    void draw(const Platform &platform);
    void draw(const HPPotion &potion);
    void draw(const FireProjectile &projectile);
    void draw(const IceProjectile &projectile);
    void draw(const Meteor &meteor);

private:
    void drawBounds(const sf::FloatRect &bounds, const sf::Color &color) const;

    sf::RenderWindow &window;
    static constexpr float OUTLINE_THICKNESS = 2.f;
};
