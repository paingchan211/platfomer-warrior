#pragma once

#include "EntityVisitor.h"
#include <SFML/Graphics.hpp>

class UISystem;
class ResourceManager;
class GameMaster;

// Visitor responsible for rendering all entities
class RenderVisitor : public EntityVisitor
{
public:
    // Constructor links window, UI, and resource managers
    RenderVisitor(sf::RenderWindow &window, UISystem &uiSystem, ResourceManager &resourceManager, GameMaster &gameMaster);

    // Visit methods for all entity types
    void visit(const Player &player) override;
    void visit(const Enemy &enemy) override;
    void visit(const Boss &boss) override;
    void visit(const Platform &platform) override;
    void visit(const HPPotion &potion) override;
    void visit(const FireProjectile &projectile) override;
    void visit(const IceProjectile &projectile) override;
    void visit(const Meteor &meteor) override;

private:
    sf::RenderWindow &window;         // Rendering window reference
    UISystem &uiSystem;             // UI renderer
    ResourceManager &resourceManager; // Texture and resource handler
    GameMaster &gameMaster;           // Game state reference
};
