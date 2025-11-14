#pragma once

#include <SFML/Graphics.hpp>

class UISystem;
class ResourceManager;
class GameMaster;
class Player;
class Enemy;
class Boss;
class Platform;
class HPPotion;
class FireProjectile;
class IceProjectile;
class Meteor;

// Centralized renderer that knows how to draw each concrete entity type
class EntityRenderer
{
public:
    EntityRenderer(sf::RenderWindow &window, UISystem &uiSystem, ResourceManager &resourceManager, GameMaster &gameMaster);

    void draw(const Player &player);
    void draw(const Enemy &enemy);
    void draw(const Boss &boss);
    void draw(const Platform &platform);
    void draw(const HPPotion &potion);
    void draw(const FireProjectile &projectile);
    void draw(const IceProjectile &projectile);
    void draw(const Meteor &meteor);

private:
    sf::RenderWindow &window;       // Rendering window reference
    UISystem &uiSystem;             // UI renderer
    ResourceManager &resourceManager; // Texture and resource handler
    GameMaster &gameMaster;         // Game state reference
};
