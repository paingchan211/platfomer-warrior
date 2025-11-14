#pragma once

#include "Character.h"
#include "Animation.h"
#include "SkillTree.h"
#include <SFML/Graphics.hpp>

// Forward declarations
class KeyBindingManager;
class HPPotion;
class Platform;

// Player class represents the controllable character with movement, attack, and stats
class Player : public Character
{
public:
    // Struct holding data about what happens during a level-up
    struct LevelUpInfo
    {
        bool leveledUp;
        int newLevel;
        int hpGained;
        int damageGained;

        LevelUpInfo() : leveledUp(false), newLevel(0), hpGained(0), damageGained(0) {}
    };

private:
    // Animation pointers
    Animation *runAnim;     // Running animation
    Animation *idleAnim;    // Idle animation
    Animation *attack1Anim; // First attack animation
    Animation *attack2Anim; // Second attack animation

    // Animation control
    int currentFrame;     // Current frame index
    float animationTimer; // Animation frame timer
    bool isRunning;       // Running state
    bool facingRight;     // Facing direction
    bool isAttacking;     // Attack state
    int attackType;       // Attack type identifier

    // Special attack / dash
    bool isSpecialAttacking;   // Whether special attack is active
    float specialDashSpeed;    // Dash movement speed
    float specialDashDistance; // Dash travel distance
    float specialDashTraveled; // Distance already traveled

    // Speed modifier
    float speedBoost; // Running boost multiplier

    // Jump and gravity mechanics
    float gravity;      // Gravity applied each frame
    float jumpStrength; // Upward jump strength
    int maxJumps;       // Maximum allowed jumps
    int jumpCount;      // Current number of jumps performed
    bool spacePressed;  // Prevents jump spamming

    // Collision and sprite setup
    float characterHeight;  // Character sprite height
    float margin;           // Horizontal margin
    float collisionWidth;   // Collision box width
    float collisionHeight;  // Collision box height
    float collisionOffsetX; // Horizontal collision offset
    float collisionOffsetY; // Vertical collision offset

    // Player attributes and systems
    SkillTree skillTree; // Tracks player skills and upgrades

    // Progression and inventory
    int level;
    int experience;
    int expToNextLevel;
    int hpPotions;
    int maxHpPotions;

    void initializeProgressionStats();

public:
    // Constructor to initialize animations and physics
    Player(Animation *run, Animation *idle, Animation *atk1, Animation *atk2,
           float startX, float startY, float charHeight, float grav, float jump);

    // Update player each frame
    void update(float dt) override;

    // Handle keyboard input (movement / attacks)
    void handleInput(float dt, const KeyBindingManager *keyManager = nullptr);

    // Handle sprite animation transitions
    void updateAnimation(float dt);

    // Apply gravity and collision logic
    void updatePhysics(float dt, float groundLevel, Platform **platforms, int numPlatforms);

    // Bounding boxes
    sf::FloatRect getBounds() const override;
    sf::FloatRect getCollisionBounds() const;
    sf::FloatRect getAttackRange() const;

    // Accessors for subsystems
    SkillTree &getSkillTree();
    const SkillTree &getSkillTree() const;

    // Progression helpers
    LevelUpInfo gainExperience(int exp);
    int getLevel() const;
    int getExperience() const;
    int getExpToNextLevel() const;
    void setProgression(int level, int experience, int expToNextLevel);

    // Inventory helpers
    bool addHpPotion();
    bool useHpPotion();
    int getHpPotions() const;
    int getMaxHpPotions() const;
    void setHpPotions(int count);

    // State checks
    bool isFacingRight() const;
    bool getIsAttacking() const;
    bool getIsSpecialAttacking() const;
    int getAttackType() const;

    // Collision info getters
    float getCollisionWidth() const;
    float getCollisionHeight() const;
    float getCollisionOffsetX() const;
    float getCollisionOffsetY() const;

    // Returns the currently active animation
    Animation *getCurrentAnimation() const;

    // Control direction and jumping
    void setFacingRight(bool facing);
    void resetJumpCount();

    // Attack and special move
    void attack(int type);
    void specialAttack();

    // Handle jump action
    bool jump(float dt, const KeyBindingManager *keyManager = nullptr);

    // Move player horizontally
    void move(float deltaX);

    // Apply stat upgrades after level-up
    void applyLevelUpBonuses(const LevelUpInfo &levelUpInfo);
};
