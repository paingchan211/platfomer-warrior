#include "EntityRenderer.h"
#include "Player.h"
#include "Enemy.h"
#include "Boss.h"
#include "Platform.h"
#include "HPPotion.h"
#include "FireProjectile.h"
#include "IceProjectile.h"
#include "Meteor.h"
#include "UISystem.h"
#include "ResourceManager.h"
#include "GameMaster.h"
#include "Animation.h"

// Initialize renderer with references to key managers
EntityRenderer::EntityRenderer(sf::RenderWindow &window, UISystem &uiSystem, ResourceManager &resourceManager, GameMaster &gameMaster)
    : window(window), uiSystem(uiSystem), resourceManager(resourceManager), gameMaster(gameMaster) {}

// Render player with rage-mode tint if active
void EntityRenderer::draw(const Player &player)
{
    if (gameMaster.isRageModeActive())
    {
        window.draw(UISystem::makeTintedSprite(player.getSprite(), sf::Color(255, 100, 100, 255)));
    }
    else
    {
        window.draw(player.getSprite());
    }
}

// Render enemy sprite and health/status UI
void EntityRenderer::draw(const Enemy &enemy)
{
    if (!enemy.isAlive())
    {
        window.draw(UISystem::makeTintedSprite(enemy.getSprite(), sf::Color(100, 100, 100, 160)));
        return;
    }

    if (enemy.isFrozen())
    {
        window.draw(UISystem::makeTintedSprite(enemy.getSprite(), sf::Color(150, 200, 255, 255)));
    }
    else if (enemy.getIsBurning())
    {
        window.draw(UISystem::makeTintedSprite(enemy.getSprite(), sf::Color(255, 180, 100, 255)));
    }
    else
    {
        window.draw(enemy.getSprite());
    }

    // Render health bar and effects
    uiSystem.renderEnemyHealthBar(window, enemy,
                                  sf::Vector2f(resourceManager.getEnemyAnim().frameSize));
    uiSystem.renderEnemyStatusEffects(window, enemy);
}

// Render boss with special rage and freeze effects
void EntityRenderer::draw(const Boss &boss)
{
    if (!boss.isAlive())
    {
        window.draw(UISystem::makeTintedSprite(boss.getSprite(), sf::Color(120, 120, 120, 200)));
        return;
    }

    if (boss.isFrozen())
    {
        window.draw(UISystem::makeTintedSprite(boss.getSprite(), sf::Color(150, 200, 255, 255)));
    }
    else if (boss.getIsBurning())
    {
        window.draw(UISystem::makeTintedSprite(boss.getSprite(), sf::Color(255, 180, 100, 255)));
    }
    else if (gameMaster.isBossRageModeActive())
    {
        window.draw(UISystem::makeTintedSprite(boss.getSprite(), sf::Color(255, 100, 100, 255)));
    }
    else
    {
        window.draw(boss.getSprite());
    }

    // Draw boss UI
    uiSystem.renderBossHealthBar(window, boss,
                                 sf::Vector2f(resourceManager.getBossAnim().frameSize), 2.5f);
    uiSystem.renderBossStatusEffects(window, boss, gameMaster.isBossRageModeActive());
}

// Draw platform if active
void EntityRenderer::draw(const Platform &platform)
{
    if (platform.isActive())
        window.draw(platform.getSprite());
}

// Draw health potion
void EntityRenderer::draw(const HPPotion &potion)
{
    if (potion.isActive())
    {
        window.draw(potion.getSprite());
    }
}

// Draw fire projectile
void EntityRenderer::draw(const FireProjectile &projectile)
{
    if (projectile.isActive())
    {
        window.draw(projectile.getSprite());
    }
}

// Draw ice projectile
void EntityRenderer::draw(const IceProjectile &projectile)
{
    if (projectile.isActive())
    {
        window.draw(projectile.getSprite());
    }
}

// Draw meteor projectile
void EntityRenderer::draw(const Meteor &meteor)
{
    if (meteor.isActive())
    {
        window.draw(meteor.getSprite());
    }
}
