#include "SkillTree.h"
#include <sstream>
#include <iostream>

// Constructor: initialize all pointers to nullptr and skill points to zero
SkillTree::SkillTree()
    : tree(nullptr),
      availableSkillPoints(0),
      fireProjectileNode(nullptr),
      iceProjectileNode(nullptr),
      fireDamageNode(nullptr),
      fireChargeNode(nullptr),
      fireStackNode(nullptr),
      iceDamageNode(nullptr),
      iceChargeNode(nullptr),
      iceStackNode(nullptr)
{
}

// Destructor: delete the main tree
SkillTree::~SkillTree()
{
    delete tree;
}

// Initialize the skill tree structure and setup connections
void SkillTree::initialize()
{
    // Root node: Special Attack (always available)
    SkillNode specialAttack("Special Attack", "Powerful special attack (S key) - Always available", SkillType::SPECIAL_ATTACK, 1, 0, 0, 0);
    specialAttack.isLocked = false;
    specialAttack.currentLevel = 1;
    tree = new NTree<SkillNode, 3>(specialAttack);

    // Create Fire Projectile branch
    SkillNode fireProj("Fire Projectile", "Unlock fire projectile attack (F key) - Starts with 1 charge, shoots 1 projectile", SkillType::FIRE_PROJECTILE, 1, 0, 0, 0);
    fireProjectileNode = new NTree<SkillNode, 3>(fireProj);
    tree->attachNTree(0, fireProjectileNode);

    // Create Ice Projectile branch
    SkillNode iceProj("Ice Projectile", "Unlock ice projectile attack (F key) - Starts with 1 charge, shoots 1 projectile", SkillType::ICE_PROJECTILE, 1, 0, 0, 0);
    iceProjectileNode = new NTree<SkillNode, 3>(iceProj);
    tree->attachNTree(1, iceProjectileNode);

    // Fire sub-skills
    SkillNode fireDmg("Fire Mastery", "Increase fire projectile damage (+5 damage per level, max 3)", SkillType::FIRE_DAMAGE_BOOST, 3, 5, 0, 0);
    fireDamageNode = new NTree<SkillNode, 3>(fireDmg);
    fireProjectileNode->attachNTree(0, fireDamageNode);

    SkillNode fireChg("Fire Reserves", "Increase fire ammo charges (+1 charge per level, max 2)", SkillType::FIRE_CHARGE_BOOST, 2, 0, 1, 0);
    fireChargeNode = new NTree<SkillNode, 3>(fireChg);
    fireProjectileNode->attachNTree(1, fireChargeNode);

    SkillNode fireStk("Fire Barrage", "Shoot more fire projectiles (+1 projectile per level, max 2)", SkillType::FIRE_STACK_BOOST, 2, 0, 0, 1);
    fireStackNode = new NTree<SkillNode, 3>(fireStk);
    fireProjectileNode->attachNTree(2, fireStackNode);

    // Ice sub-skills
    SkillNode iceDmg("Ice Mastery", "Increase ice projectile damage (+5 damage per level, max 3)", SkillType::ICE_DAMAGE_BOOST, 3, 5, 0, 0);
    iceDamageNode = new NTree<SkillNode, 3>(iceDmg);
    iceProjectileNode->attachNTree(0, iceDamageNode);

    SkillNode iceChg("Ice Reserves", "Increase ice ammo charges (+1 charge per level, max 2)", SkillType::ICE_CHARGE_BOOST, 2, 0, 1, 0);
    iceChargeNode = new NTree<SkillNode, 3>(iceChg);
    iceProjectileNode->attachNTree(1, iceChargeNode);

    SkillNode iceStk("Ice Storm", "Shoot more ice projectiles (+1 projectile per level, max 2)", SkillType::ICE_STACK_BOOST, 2, 0, 0, 1);
    iceStackNode = new NTree<SkillNode, 3>(iceStk);
    iceProjectileNode->attachNTree(2, iceStackNode);
}

// Add skill points to the player
void SkillTree::addSkillPoints(int points)
{
    availableSkillPoints += points;
}

// Upgrade a skill node if enough points and prerequisites are met
bool SkillTree::upgradeSkill(SkillType type)
{
    if (availableSkillPoints <= 0)
    {
        return false; // No points available
    }

    auto node = getSkillNode(type);
    if (!node)
    {
        return false; // Node not found
    }

    if (!canUpgradeSkill(type))
    {
        return false; // Upgrade not allowed
    }

    // Perform the upgrade
    node->getKey().currentLevel++;
    node->getKey().isLocked = false;
    availableSkillPoints--;

    return true;
}

// Check if a skill can be upgraded
bool SkillTree::canUpgradeSkill(SkillType type) const
{
    auto node = getSkillNode(type);
    if (!node)
        return false;

    if (type == SkillType::SPECIAL_ATTACK)
        return false; // Root skill can't be upgraded

    if (node->getKey().isMaxLevel())
        return false; // Already maxed out

    return arePrerequisitesMet(node); // Check prerequisites
}

// Retrieve a mutable skill node by type
NTree<SkillNode, 3> *SkillTree::getSkillNode(SkillType type)
{
    if (!tree)
        return nullptr;

    SkillNode searchNode;
    searchNode.type = type;

    return tree->search(searchNode);
}

// Retrieve a constant skill node by type
const NTree<SkillNode, 3> *SkillTree::getSkillNode(SkillType type) const
{
    if (!tree)
        return nullptr;

    SkillNode searchNode;
    searchNode.type = type;

    return tree->search(searchNode);
}

// Calculate total fire damage from unlocked fire skills
int SkillTree::getTotalFireDamage() const
{
    if (!hasFireProjectile())
        return 0;

    int total = 0;

    if (fireDamageNode && !fireDamageNode->isEmpty() && fireDamageNode->getKey().isUnlocked())
    {
        total += fireDamageNode->getKey().getTotalDamageBonus();
    }

    return total;
}

// Calculate total ice damage from unlocked ice skills
int SkillTree::getTotalIceDamage() const
{
    if (!hasIceProjectile())
        return 0;

    int total = 0;

    if (iceDamageNode && !iceDamageNode->isEmpty() && iceDamageNode->getKey().isUnlocked())
    {
        total += iceDamageNode->getKey().getTotalDamageBonus();
    }

    return total;
}

// Get total number of fire projectile charges
int SkillTree::getFireProjectileCharges() const
{
    if (!hasFireProjectile())
        return 0;

    int charges = 1;

    if (fireChargeNode && !fireChargeNode->isEmpty() && fireChargeNode->getKey().isUnlocked())
    {
        charges += fireChargeNode->getKey().getTotalChargeBonus();
    }

    return charges;
}

// Get total number of ice projectile charges
int SkillTree::getIceProjectileCharges() const
{
    if (!hasIceProjectile())
        return 0;

    int charges = 1;

    if (iceChargeNode && !iceChargeNode->isEmpty() && iceChargeNode->getKey().isUnlocked())
    {
        charges += iceChargeNode->getKey().getTotalChargeBonus();
    }

    return charges;
}

// Get total number of fire projectiles fired
int SkillTree::getFireProjectileCount() const
{
    if (!hasFireProjectile())
        return 0;

    int count = 1;

    if (fireStackNode && !fireStackNode->isEmpty() && fireStackNode->getKey().isUnlocked())
    {
        count += fireStackNode->getKey().getTotalStackBonus();
    }

    return count;
}

// Get total number of ice projectiles fired
int SkillTree::getIceProjectileCount() const
{
    if (!hasIceProjectile())
        return 0;

    int count = 1;

    if (iceStackNode && !iceStackNode->isEmpty() && iceStackNode->getKey().isUnlocked())
    {
        count += iceStackNode->getKey().getTotalStackBonus();
    }

    return count;
}

// Check if the player has unlocked fire projectile skill
bool SkillTree::hasFireProjectile() const
{
    return fireProjectileNode && !fireProjectileNode->isEmpty() && fireProjectileNode->getKey().isUnlocked();
}

// Check if the player has unlocked ice projectile skill
bool SkillTree::hasIceProjectile() const
{
    return iceProjectileNode && !iceProjectileNode->isEmpty() && iceProjectileNode->getKey().isUnlocked();
}

// Reset all skill levels except the root node
void SkillTree::reset()
{
    availableSkillPoints = 0;

    if (tree)
    {
        tree->traverseLevelOrder([](SkillNode &skill)
                                  {
            if (skill.type != SkillType::SPECIAL_ATTACK)
            {
                skill.currentLevel = 0;
                skill.isLocked = true;
            } });
    }
}

// Check if all prerequisite skills are unlocked
bool SkillTree::arePrerequisitesMet(const NTree<SkillNode, 3> *node) const
{
    if (!node || node->isEmpty())
        return false;

    if (node->isRoot())
        return true;

    auto parent = node->getParent();
    if (parent && !parent->isEmpty() && parent->getKey().isUnlocked())
        return true;

    return false;
}

// Return total available skill points
int SkillTree::getSkillPoints() const
{
    return availableSkillPoints;
}

// Return total number of nodes in the tree
int SkillTree::getTreeSize() const
{
    if (!tree)
        return 0;
    return tree->getSize();
}

// Clear an entire branch (fire or ice) and refund points
void SkillTree::clearSkillBranch(SkillType branchRoot)
{
    if (!tree)
        return;

    // Handle Fire branch reset
    if (branchRoot == SkillType::FIRE_PROJECTILE)
    {
        if (!fireProjectileNode || fireProjectileNode->isEmpty())
            return;

        int pointsRefunded = 0;

        // Traverse and refund points
        fireProjectileNode->traverseLevelOrder([&pointsRefunded](SkillNode &skill)
                                                {
            int points = skill.currentLevel;
            if (points > 0 && skill.type != SkillType::FIRE_PROJECTILE)
            {
                pointsRefunded += points;
            } });

        availableSkillPoints += pointsRefunded;

        // Rebuild Fire branch
        for (int i = 0; i < 3; ++i)
        {
            if (&((*tree)[i]) == fireProjectileNode)
            {
                NTree<SkillNode, 3> *detached = tree->detachNTree(i);
                delete detached;

                SkillNode fireProj("Fire Projectile", "Unlock fire projectile attack (F key) - Starts with 1 charge, shoots 1 projectile", SkillType::FIRE_PROJECTILE, 1, 0, 0, 0);
                fireProj.isLocked = false;
                fireProj.currentLevel = 1;
                fireProjectileNode = new NTree<SkillNode, 3>(fireProj);
                tree->attachNTree(i, fireProjectileNode);

                SkillNode fireDmg("Fire Mastery", "Increase fire projectile damage (+5 damage per level, max 3)", SkillType::FIRE_DAMAGE_BOOST, 3, 5, 0, 0);
                fireDamageNode = new NTree<SkillNode, 3>(fireDmg);
                fireProjectileNode->attachNTree(0, fireDamageNode);

                SkillNode fireChg("Fire Reserves", "Increase fire ammo charges (+1 charge per level, max 2)", SkillType::FIRE_CHARGE_BOOST, 2, 0, 1, 0);
                fireChargeNode = new NTree<SkillNode, 3>(fireChg);
                fireProjectileNode->attachNTree(1, fireChargeNode);

                SkillNode fireStk("Fire Barrage", "Shoot more fire projectiles (+1 projectile per level, max 2)", SkillType::FIRE_STACK_BOOST, 2, 0, 0, 1);
                fireStackNode = new NTree<SkillNode, 3>(fireStk);
                fireProjectileNode->attachNTree(2, fireStackNode);

                break;
            }
        }
    }
    // Handle Ice branch reset
    else if (branchRoot == SkillType::ICE_PROJECTILE)
    {
        if (!iceProjectileNode || iceProjectileNode->isEmpty())
            return;

        int pointsRefunded = 0;

        // Traverse and refund points
        iceProjectileNode->traverseLevelOrder([&pointsRefunded](SkillNode &skill)
                                               {
            int points = skill.currentLevel;
            if (points > 0 && skill.type != SkillType::ICE_PROJECTILE)
            {
                pointsRefunded += points;
            } });

        availableSkillPoints += pointsRefunded;

        // Rebuild Ice branch
        for (int i = 0; i < 3; ++i)
        {
            if (&((*tree)[i]) == iceProjectileNode)
            {
                NTree<SkillNode, 3> *detached = tree->detachNTree(i);
                delete detached;

                SkillNode iceProj("Ice Projectile", "Unlock ice projectile attack (F key) - Starts with 1 charge, shoots 1 projectile", SkillType::ICE_PROJECTILE, 1, 0, 0, 0);
                iceProj.isLocked = false;
                iceProj.currentLevel = 1;
                iceProjectileNode = new NTree<SkillNode, 3>(iceProj);
                tree->attachNTree(i, iceProjectileNode);

                SkillNode iceDmg("Ice Mastery", "Increase ice projectile damage (+5 damage per level, max 3)", SkillType::ICE_DAMAGE_BOOST, 3, 5, 0, 0);
                iceDamageNode = new NTree<SkillNode, 3>(iceDmg);
                iceProjectileNode->attachNTree(0, iceDamageNode);

                SkillNode iceChg("Ice Reserves", "Increase ice ammo charges (+1 charge per level, max 2)", SkillType::ICE_CHARGE_BOOST, 2, 0, 1, 0);
                iceChargeNode = new NTree<SkillNode, 3>(iceChg);
                iceProjectileNode->attachNTree(1, iceChargeNode);

                SkillNode iceStk("Ice Storm", "Shoot more ice projectiles (+1 projectile per level, max 2)", SkillType::ICE_STACK_BOOST, 2, 0, 0, 1);
                iceStackNode = new NTree<SkillNode, 3>(iceStk);
                iceProjectileNode->attachNTree(2, iceStackNode);

                break;
            }
        }
    }
}