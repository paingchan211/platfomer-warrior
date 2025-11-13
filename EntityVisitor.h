#pragma once

// Forward declarations for all entity types that can be visited
class Player;
class Enemy;
class Boss;
class Platform;
class HPPotion;
class FireProjectile;
class IceProjectile;
class Meteor;
class Entity;

// Interface for the Visitor pattern to operate on different Entity types
class EntityVisitor
{
public:
    virtual ~EntityVisitor() = default; // Virtual destructor for safe cleanup

    // Visit functions for each entity type
    virtual void visit(const Player &player) = 0;
    virtual void visit(const Enemy &enemy) = 0;
    virtual void visit(const Boss &boss) = 0;
    virtual void visit(const Platform &platform) = 0;
    virtual void visit(const HPPotion &potion) = 0;
    virtual void visit(const FireProjectile &projectile) = 0;
    virtual void visit(const IceProjectile &projectile) = 0;
    virtual void visit(const Meteor &meteor) = 0;
};
