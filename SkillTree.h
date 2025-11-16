#pragma once

#include "Tree.h"
#include <string>
#include <functional>

// Enumeration of skill types
enum class SkillType
{
    SPECIAL_ATTACK,
    FIRE_PROJECTILE,
    ICE_PROJECTILE,
    FIRE_DAMAGE_BOOST,
    FIRE_CHARGE_BOOST,
    FIRE_STACK_BOOST,
    ICE_DAMAGE_BOOST,
    ICE_CHARGE_BOOST,
    ICE_STACK_BOOST
};

// Structure representing a skill node in the tree
struct SkillNode
{
    std::string name;        // Skill name
    std::string description; // Description of the skill
    SkillType type;          // Type of skill

    bool isLocked;    // Whether the skill is locked
    int currentLevel; // Current level of skill
    int maxLevel;     // Maximum level allowed

    int damageBonus;         // Damage bonus per level
    int chargeBonus;         // Charge bonus per level
    int stackBonus;          // Stack bonus per level
    float cooldownReduction; // Cooldown reduction

    // Default constructor
    SkillNode()
        : name("Unknown"),
          description(""),
          type(SkillType::SPECIAL_ATTACK),
          isLocked(true),
          currentLevel(0),
          maxLevel(1),
          damageBonus(0),
          chargeBonus(0),
          stackBonus(0),
          cooldownReduction(0.0f)
    {
    }

    // Parameterized constructor
    SkillNode(const std::string &skillName,
              const std::string &desc,
              SkillType skillType,
              int maxLvl = 3,
              int dmgBonus = 0,
              int chgBonus = 0,
              int stkBonus = 0)
        : name(skillName),
          description(desc),
          type(skillType),
          isLocked(true),
          currentLevel(0),
          maxLevel(maxLvl),
          damageBonus(dmgBonus),
          chargeBonus(chgBonus),
          stackBonus(stkBonus),
          cooldownReduction(0.0f)
    {
    }

    // Check if skill reached maximum level
    bool isMaxLevel() const { return currentLevel >= maxLevel; }

    // Check if skill is unlocked
    bool isUnlocked() const { return currentLevel > 0; }

    // Calculate total damage bonus
    int getTotalDamageBonus() const { return damageBonus * currentLevel; }

    // Calculate total charge bonus
    int getTotalChargeBonus() const { return chargeBonus * currentLevel; }

    // Calculate total stack bonus
    int getTotalStackBonus() const { return stackBonus * currentLevel; }

    // Equality operator (compare by type)
    bool operator==(const SkillNode &other) const { return type == other.type; }
};

// Class managing a skill tree system
class SkillTree
{
private:
    NTree<SkillNode, 3> *tree; // Root of the skill tree
    int availableSkillPoints;  // Available skill points

    // Pointers to key skill branches
    NTree<SkillNode, 3> *fireProjectileNode;
    NTree<SkillNode, 3> *iceProjectileNode;
    NTree<SkillNode, 3> *fireDamageNode;
    NTree<SkillNode, 3> *fireChargeNode;
    NTree<SkillNode, 3> *fireStackNode;
    NTree<SkillNode, 3> *iceDamageNode;
    NTree<SkillNode, 3> *iceChargeNode;
    NTree<SkillNode, 3> *iceStackNode;

public:
    SkillTree();  // Constructor
    ~SkillTree(); // Destructor

    void initialize();               // Initialize tree with skills
    void addSkillPoints(int points); // Add skill points
    int getSkillPoints() const;      // Get total available skill points

    bool upgradeSkill(SkillType type);          // Upgrade skill if possible
    bool canUpgradeSkill(SkillType type) const; // Check if upgrade allowed

    NTree<SkillNode, 3> *getSkillNode(SkillType type);             // Get mutable node
    const NTree<SkillNode, 3> *getSkillNode(SkillType type) const; // Get const node

    int getTreeSize() const; // Get size of tree

    int getTotalFireDamage() const; // Get total fire damage
    int getTotalIceDamage() const;  // Get total ice damage

    int getFireProjectileCharges() const; // Get fire projectile charges
    int getIceProjectileCharges() const;  // Get ice projectile charges

    int getFireProjectileCount() const; // Get number of fire projectiles
    int getIceProjectileCount() const;  // Get number of ice projectiles

    bool hasSpecialAttack() const;   // Check if special attack unlocked
    bool hasFireProjectile() const; // Check if fire projectile unlocked
    bool hasIceProjectile() const;  // Check if ice projectile unlocked

    void reset();                                // Reset all skill progress
    void clearSkillBranch(SkillType branchRoot); // Reset a branch

private:
    bool arePrerequisitesMet(const NTree<SkillNode, 3> *node) const; // Check parent unlock
};
