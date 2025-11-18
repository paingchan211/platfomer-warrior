#include "UISystem.h"
#include "SaveGameManager.h"
#include "SkillTree.h"
#include <cmath>
#include <string>
#include <algorithm>

// Constructor initializes shared UI state
UISystem::UISystem(ResourceManager &resourceManager)
    : resourceManager(resourceManager),
      toastMessage(""),
      toastTimer(0.0f),
      toastColor(sf::Color::White)
{
}

// Destructor
UISystem::~UISystem()
{
}

// Render the Game Over or Victory screen
void UISystem::renderGameOverScreen(sf::RenderWindow &window,
                                    bool gameWon,
                                    const Player &player,
                                    const std::unique_ptr<std::unique_ptr<Enemy>[]> &enemies,
                                    std::size_t enemyCount,
                                    const DoublyLinkedList<std::unique_ptr<FireProjectile>> &fireProjectiles,
                                    const DoublyLinkedList<std::unique_ptr<HPPotion>> &hpPotions,
                                    const DoublyLinkedList<std::unique_ptr<Meteor>> &meteors,
                                    const std::unique_ptr<sf::Sprite[]> &floorTiles,
                                    std::size_t floorTileCount,
                                    const std::unique_ptr<std::unique_ptr<Platform>[]> &platforms,
                                    std::size_t platformCount)
{
    window.clear();

    // Draw floor tiles
    if (floorTiles)
    {
        for (std::size_t i = 0; i < floorTileCount; ++i)
            window.draw(floorTiles[i]);
    }

    // Draw active platforms
    if (platforms)
    {
        for (std::size_t i = 0; i < platformCount; ++i)
            if (platforms[i] && platforms[i]->isActive())
                window.draw(platforms[i]->getSprite());
    }

    // Draw enemies (tinted if dead)
    if (enemies)
    {
        for (std::size_t i = 0; i < enemyCount; ++i)
        {
            if (enemies[i])
            {
                // Skip entirely inactive enemies (those that have been removed/deactivated)
                if (!enemies[i]->isActive())
                    continue;
                if (enemies[i]->isAlive())
                    window.draw(enemies[i]->getSprite());
                else
                    window.draw(makeTintedSprite(enemies[i]->getSprite(), sf::Color(100, 100, 100, 180)));
            }
        }
    }

    // Draw projectiles, potions, and meteors if active
    fireProjectiles.forEach([&window](const std::unique_ptr<FireProjectile> &proj)
                            {
    if (proj && proj->isActive())
        window.draw(proj->getSprite()); }); // Draw active fire projectiles

    hpPotions.forEach([&window](const std::unique_ptr<HPPotion> &potion)
                      {
    if (potion && potion->isActive())
        window.draw(potion->getSprite()); }); // Draw active HP potions

    meteors.forEach([&window](const std::unique_ptr<Meteor> &meteor)
                    {
    if (meteor && meteor->isActive())
        window.draw(meteor->getSprite()); }); // Draw active meteors

    // Tint the player sprite
    window.draw(makeTintedSprite(player.getSprite(), sf::Color(160, 160, 160, 220)));

    // Overlay transparent dark layer
    window.setView(window.getDefaultView());
    window.draw(makeBar({0.f, 0.f}, {SCREEN_WIDTH, SCREEN_HEIGHT}, sf::Color(0, 0, 0, 170), sf::Color::Transparent, 0.f));

    // Draw banner background
    sf::Sprite banner(resourceManager.getTexture("banner"));
    banner.setScale({6.f, 3.f});
    const float bannerWidth = banner.getGlobalBounds().width;
    const float bannerHeight = banner.getGlobalBounds().height;
    banner.setPosition({(SCREEN_WIDTH - bannerWidth) / 2.f, (SCREEN_HEIGHT - bannerHeight) / 2.f - 50.f});
    window.draw(banner);

    // Draw title ("GAME OVER" or "CONGRATULATIONS!")
    const std::string title = gameWon ? "CONGRATULATIONS!" : "GAME OVER";
    sf::Text titleText(title, resourceManager.getFont(), 40);
    const sf::FloatRect titleBounds = titleText.getLocalBounds();
    titleText.setPosition({(SCREEN_WIDTH - titleBounds.width) / 2.f, (SCREEN_HEIGHT - titleBounds.height) / 2.f - 60.f});
    titleText.setFillColor(sf::Color::Black);
    window.draw(titleText);

    // Draw subtitle if won
    const std::string subtitle = gameWon ? "You defeated the boss!" : "";
    if (!subtitle.empty())
    {
        sf::Text subText(subtitle, resourceManager.getFont(), 24);
        const sf::FloatRect subBounds = subText.getLocalBounds();
        subText.setPosition({(SCREEN_WIDTH - subBounds.width) / 2.f, (SCREEN_HEIGHT - subBounds.height) / 2.f - 20.f});
        subText.setFillColor(sf::Color::Black);
        window.draw(subText);
    }

    // Restart prompt
    sf::Text restartText("Press R to restart", resourceManager.getFont(), 24);
    const sf::FloatRect restartBounds = restartText.getLocalBounds();
    restartText.setPosition({(SCREEN_WIDTH - restartBounds.width) / 2.f, (SCREEN_HEIGHT - restartBounds.height) / 2.f + 20.f});
    restartText.setFillColor(sf::Color::Black);
    window.draw(restartText);
}

// Draw player sprite with optional rage tint
void UISystem::renderPlayerEntity(sf::RenderWindow &window, const Player &player, bool rageModeActive)
{
    if (!player.isActive())
        return;

    if (rageModeActive)
    {
        window.draw(UISystem::makeTintedSprite(player.getSprite(), sf::Color(255, 100, 100, 255)));
    }
    else
    {
        window.draw(player.getSprite());
    }
}

// Draw enemy sprite along with per-enemy UI
void UISystem::renderEnemyEntity(sf::RenderWindow &window, const Enemy &enemy)
{
    if (!enemy.isActive())
        return;

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

    renderEnemyHealthBar(window, enemy, sf::Vector2f(resourceManager.getEnemyAnim().frameSize));
    renderEnemyStatusEffects(window, enemy);
}

// Draw boss sprite along with boss UI
void UISystem::renderBossEntity(sf::RenderWindow &window, const Boss &boss, bool bossRageModeActive)
{
    if (!boss.isActive())
        return;

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
    else if (bossRageModeActive)
    {
        window.draw(UISystem::makeTintedSprite(boss.getSprite(), sf::Color(255, 100, 100, 255)));
    }
    else
    {
        window.draw(boss.getSprite());
    }

    renderBossHealthBar(window, boss, sf::Vector2f(resourceManager.getBossAnim().frameSize), 2.5f);
    renderBossStatusEffects(window, boss, bossRageModeActive);
}

// Draw a platform if active
void UISystem::renderPlatformEntity(sf::RenderWindow &window, const Platform &platform)
{
    if (platform.isActive())
    {
        window.draw(platform.getSprite());
    }
}

// Draw a HP potion if active
void UISystem::renderHPPotionEntity(sf::RenderWindow &window, const HPPotion &potion)
{
    if (potion.isActive())
    {
        window.draw(potion.getSprite());
    }
}

// Draw fire projectile if active
void UISystem::renderFireProjectileEntity(sf::RenderWindow &window, const FireProjectile &projectile)
{
    if (projectile.isActive())
    {
        window.draw(projectile.getSprite());
    }
}

// Draw ice projectile if active
void UISystem::renderIceProjectileEntity(sf::RenderWindow &window, const IceProjectile &projectile)
{
    if (projectile.isActive())
    {
        window.draw(projectile.getSprite());
    }
}

// Draw meteor if active
void UISystem::renderMeteorEntity(sf::RenderWindow &window, const Meteor &meteor)
{
    if (meteor.isActive())
    {
        window.draw(meteor.getSprite());
    }
}

// Render the HUD overlay (health, XP, ammo, etc.)
void UISystem::renderHUD(sf::RenderWindow &window,
                         const Player &player,
                         int projectileAmmo,
                         float projectileCooldown,
                         int specialAttackAmmo,
                         float specialAttackCooldown,
                         bool isFireProjectile,
                         bool rageMode,
                         bool meteorFuryMode,
                         bool bossRageMode)
{
    window.setView(window.getDefaultView());
    // Draw core HUD components
    renderHealthBar(window, player);
    renderXPBar(window, player);
    renderInventoryDisplay(window, player);
    renderProjectileAmmo(window, projectileAmmo, projectileCooldown, player, isFireProjectile);
    const bool hasSpecialAttack = player.getSkillTree().hasSpecialAttack();
    renderSpecialAttackAmmo(window, specialAttackAmmo, specialAttackCooldown, hasSpecialAttack);
    renderControlsText(window);

    // Alerts for special modes (Rage, Meteor, Boss Rage)
    float yOffset = 120.f;

    if (rageMode)
    {
        renderCenteredAlert(window, "RAGE MODE! FIGHT FOR YOUR LIFE! DOUBLE DAMAGE!",
                            yOffset, sf::Color(255, 50, 0), sf::Color(255, 50, 0));
        yOffset += 70.f;
    }

    if (bossRageMode)
    {
        renderCenteredAlert(window, "THE BOSS IS ENRAGED! DOUBLE DAMAGE!",
                            yOffset, sf::Color(255, 0, 0), sf::Color(200, 0, 0));
        yOffset += 70.f;
    }

    if (meteorFuryMode)
    {
        renderCenteredAlert(window, "The boss summons a meteor storm in desperation!",
                            yOffset, sf::Color(255, 140, 0), sf::Color(255, 140, 0));
    }
}

// Render player's HP bar
void UISystem::renderHealthBar(sf::RenderWindow &window, const Player &player)
{
    const sf::Vector2f hpBarPos{10.f, 10.f};
    const sf::Vector2f barSize{200.f, 20.f};

    // Draw border
    window.draw(makeBar(hpBarPos, barSize, sf::Color(40, 40, 40), sf::Color::White, 2.f));

    // Draw filled portion
    const float hpRatio = static_cast<float>(player.getHp()) / static_cast<float>(player.getMaxHp());
    window.draw(makeBar(hpBarPos, {barSize.x * hpRatio, barSize.y}, sf::Color::Red, sf::Color::Transparent, 0.f));

    // Draw text label
    sf::Text hpText("HP: " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()),
                    resourceManager.getFont(), 18);
    hpText.setFillColor(sf::Color::White);
    hpText.setPosition({hpBarPos.x + barSize.x + 12.f, hpBarPos.y});
    window.draw(hpText);
}

// Render XP bar
void UISystem::renderXPBar(sf::RenderWindow &window, const Player &player)
{
    const sf::Vector2f hpBarPos{10.f, 10.f};
    const sf::Vector2f barSize{200.f, 20.f};
    const sf::Vector2f xpBarPos{10.f, hpBarPos.y + barSize.y + 8.f};

    // Background bar
    window.draw(makeBar(xpBarPos, barSize, sf::Color(40, 40, 40), sf::Color::White, 2.f));

    // XP fill
    const float xpRatio = static_cast<float>(player.getExperience()) / static_cast<float>(player.getExpToNextLevel());
    window.draw(makeBar(xpBarPos, {barSize.x * xpRatio, barSize.y}, sf::Color::Blue, sf::Color::Transparent, 0.f));

    // XP text
    sf::Text xpText("Level " + std::to_string(player.getLevel()) +
                        " - XP: " + std::to_string(player.getExperience()) + "/" + std::to_string(player.getExpToNextLevel()),
                    resourceManager.getFont(), 18);
    xpText.setFillColor(sf::Color::White);
    xpText.setPosition({xpBarPos.x + barSize.x + 12.f, xpBarPos.y});
    window.draw(xpText);
}

// Render player inventory (HP potions count)
void UISystem::renderInventoryDisplay(sf::RenderWindow &window, const Player &player)
{
    const sf::Vector2f hpBarPos{10.f, 10.f};
    const sf::Vector2f barSize{200.f, 20.f};
    const sf::Vector2f xpBarPos{10.f, hpBarPos.y + barSize.y + 8.f};

    sf::Text inventoryText("HP Potions: " + std::to_string(player.getHpPotions()) + "/" +
                               std::to_string(player.getMaxHpPotions()) + " (Press Q to use)",
                           resourceManager.getFont(), 18);
    inventoryText.setFillColor(sf::Color::White);
    inventoryText.setPosition({hpBarPos.x, xpBarPos.y + barSize.y + 10.f});
    window.draw(inventoryText);
}

// Render circular indicator for projectile ammo
void UISystem::renderProjectileAmmo(sf::RenderWindow &window, int ammo, float cooldown, const Player &player, bool isFireProjectile)
{
    const SkillTree &skills = player.getSkillTree();
    if (skills.hasFireProjectile() || skills.hasIceProjectile())
    {
        int maxAmmo = isFireProjectile ? skills.getFireProjectileCharges() : skills.getIceProjectileCharges();

        // Create circular UI element
        sf::CircleShape projectileCircle(30.f);
        const sf::Vector2f circleCenter{SCREEN_WIDTH - 130.f, SCREEN_HEIGHT - 50.f};
        projectileCircle.setPosition(circleCenter - sf::Vector2f{projectileCircle.getRadius(), projectileCircle.getRadius()});

        // Color based on ammo state
        const sf::Color ammoColor = ammo == maxAmmo
                                        ? sf::Color(0, 200, 0, 160)
                                        : (ammo > 0 ? sf::Color(200, 200, 0, 160) : sf::Color(200, 0, 0, 160));
        projectileCircle.setFillColor(ammoColor);

        const sf::Color borderColor = isFireProjectile ? sf::Color(255, 100, 0) : sf::Color(100, 200, 255);
        projectileCircle.setOutlineColor(borderColor);
        projectileCircle.setOutlineThickness(3.f);
        window.draw(projectileCircle);

        // Cooldown progress overlay
        if (ammo < maxAmmo && cooldown > 0.f)
        {
            const float progress = 1.f - cooldown / PROJECTILE_COOLDOWN_TIME;
            sf::VertexArray arc(sf::TriangleFan, 34);
            arc[0].position = circleCenter;
            arc[0].color = sf::Color(255, 255, 255, 90);
            for (int i = 0; i <= 32; ++i)
            {
                const float angle = (-90.f + 360.f * progress * (static_cast<float>(i) / 32.f)) * 3.1415926f / 180.f;
                arc[i + 1].position = circleCenter + sf::Vector2f(std::cos(angle), std::sin(angle)) * projectileCircle.getRadius() * 0.8f;
                arc[i + 1].color = sf::Color(255, 255, 255, 90);
            }
            window.draw(arc);
        }

        // Label and text
        sf::Text projectileLabel("F", resourceManager.getFont(), 22);
        const sf::FloatRect projectileBounds = projectileLabel.getLocalBounds();
        projectileLabel.setPosition(circleCenter.x - projectileBounds.width / 2.f, circleCenter.y - projectileBounds.height);
        window.draw(projectileLabel);

        std::string typeText = isFireProjectile ? "Fire" : "Ice";
        sf::Text typeLabel(typeText, resourceManager.getFont(), 12);
        const sf::FloatRect typeBounds = typeLabel.getLocalBounds();
        typeLabel.setPosition(circleCenter.x - typeBounds.width / 2.f, circleCenter.y - 35.f);
        typeLabel.setFillColor(borderColor);
        window.draw(typeLabel);

        sf::Text ammoText(std::to_string(ammo) + "/" + std::to_string(maxAmmo), resourceManager.getFont(), 14);
        const sf::FloatRect ammoBounds = ammoText.getLocalBounds();
        ammoText.setPosition(circleCenter.x - ammoBounds.width / 2.f, circleCenter.y + 8.f);
        window.draw(ammoText);
    }
}

// Render special attack ammo indicator
void UISystem::renderSpecialAttackAmmo(sf::RenderWindow &window, int ammo, float cooldown, bool unlocked)
{
    sf::CircleShape specialCircle(30.f);
    const sf::Vector2f circleCenter{SCREEN_WIDTH - 50.f, SCREEN_HEIGHT - 50.f};
    specialCircle.setPosition(circleCenter - sf::Vector2f{specialCircle.getRadius(), specialCircle.getRadius()});

    if (unlocked)
    {
        const sf::Color ammoColor = ammo == MAX_SPECIAL_ATTACK_AMMO
                                        ? sf::Color(0, 200, 0, 160)
                                        : sf::Color(200, 0, 0, 160);
        specialCircle.setFillColor(ammoColor);
        specialCircle.setOutlineColor(sf::Color::White);
        specialCircle.setOutlineThickness(2.f);
    }
    else
    {
        specialCircle.setFillColor(sf::Color(40, 40, 40, 160));
        specialCircle.setOutlineColor(sf::Color(120, 120, 120));
        specialCircle.setOutlineThickness(1.5f);
    }
    window.draw(specialCircle);

    // Cooldown progress overlay (only if unlocked)
    if (unlocked && ammo < MAX_SPECIAL_ATTACK_AMMO && cooldown > 0.f && SPECIAL_ATTACK_COOLDOWN_TIME > 0.f)
    {
        const float progress = 1.f - cooldown / SPECIAL_ATTACK_COOLDOWN_TIME;
        sf::VertexArray arc(sf::TriangleFan, 34);
        arc[0].position = circleCenter;
        arc[0].color = sf::Color(255, 255, 255, 90);
        for (int i = 0; i <= 32; ++i)
        {
            const float angle = (-90.f + 360.f * progress * (static_cast<float>(i) / 32.f)) * 3.1415926f / 180.f;
            arc[i + 1].position = circleCenter + sf::Vector2f(std::cos(angle), std::sin(angle)) * specialCircle.getRadius() * 0.8f;
            arc[i + 1].color = sf::Color(255, 255, 255, 90);
        }
        window.draw(arc);
    }

    // Labels
    sf::Text specialLabel("S", resourceManager.getFont(), 22);
    specialLabel.setFillColor(unlocked ? sf::Color::White : sf::Color(160, 160, 160));
    const sf::FloatRect specialBounds = specialLabel.getLocalBounds();
    specialLabel.setPosition(circleCenter.x - specialBounds.width / 2.f, circleCenter.y - specialBounds.height);
    window.draw(specialLabel);

    sf::Text ammoText(unlocked ? std::to_string(ammo) + "/" + std::to_string(MAX_SPECIAL_ATTACK_AMMO) : "LOCKED",
                      resourceManager.getFont(),
                      unlocked ? 14 : 12);
    ammoText.setFillColor(unlocked ? sf::Color::White : sf::Color(200, 200, 200));
    const sf::FloatRect ammoBounds = ammoText.getLocalBounds();
    ammoText.setPosition(circleCenter.x - ammoBounds.width / 2.f, circleCenter.y + 8.f);
    window.draw(ammoText);
}

// Render on-screen control instructions
void UISystem::renderControlsText(sf::RenderWindow &window)
{
    sf::Text controlsText("Controls: Arrow Keys move, Space jump, A/S attack, F projectile, R switch",
                          resourceManager.getFont(), 14);
    controlsText.setFillColor(sf::Color(210, 210, 210));
    controlsText.setPosition({10.f, SCREEN_HEIGHT - 24.f});
    window.draw(controlsText);
}

// Render enemy HP bar above its sprite
void UISystem::renderEnemyHealthBar(sf::RenderWindow &window,
                                    const Enemy &enemy,
                                    const sf::Vector2f &frameSize)
{
    if (enemy.getHp() >= enemy.getMaxHp())
        return;

    const float enemyHpBarWidth = 60.f;
    const float enemyHpBarHeight = 6.f;
    const sf::Vector2f enemyPos = enemy.getPosition();
    const sf::Vector2f enemyBarPos{enemyPos.x + (frameSize.x - enemyHpBarWidth) / 2.f,
                                   enemyPos.y - 15.f};

    // Draw bar background
    window.draw(makeBar(enemyBarPos, {enemyHpBarWidth, enemyHpBarHeight}, sf::Color(40, 40, 40), sf::Color::White, 1.f));

    // Draw current HP portion
    const float enemyHpRatio = static_cast<float>(enemy.getHp()) / static_cast<float>(enemy.getMaxHp());
    sf::Color healthColor = enemyHpRatio > 0.6f ? sf::Color::Green : (enemyHpRatio > 0.3f ? sf::Color::Yellow : sf::Color::Red);
    window.draw(makeBar(enemyBarPos, {enemyHpBarWidth * enemyHpRatio, enemyHpBarHeight}, healthColor, sf::Color::Transparent, 0.f));
}

// Render enemy status effects (Frozen, Burning)
void UISystem::renderEnemyStatusEffects(sf::RenderWindow &window,
                                        const Enemy &enemy)
{
    sf::FloatRect bounds = enemy.getCollisionBounds();

    // Frozen or Slow indicator
    if (enemy.getIceStacks() > 0)
    {
        sf::Vector2f indicatorPos(bounds.left + bounds.width / 2.f - 15.f, bounds.top - 30.f);
        std::string text = enemy.isFrozen() ? "FROZEN" : "SLOW";
        sf::Text statusText(text, resourceManager.getFont(), 12);
        statusText.setPosition(indicatorPos);
        statusText.setFillColor(sf::Color::Cyan);
        statusText.setOutlineColor(sf::Color::Black);
        statusText.setOutlineThickness(1.f);
        window.draw(statusText);
    }

    // Burning indicator
    if (enemy.getIsBurning())
    {
        sf::Vector2f indicatorPos(bounds.left + bounds.width / 2.f - 20.f, bounds.top - 50.f);
        sf::Text statusText("BURNING", resourceManager.getFont(), 12);
        statusText.setPosition(indicatorPos);
        statusText.setFillColor(sf::Color(255, 100, 0));
        statusText.setOutlineColor(sf::Color::Black);
        statusText.setOutlineThickness(1.f);
        window.draw(statusText);
    }
}

// Render boss HP bar above the sprite
void UISystem::renderBossHealthBar(sf::RenderWindow &window,
                                   const Boss &boss,
                                   const sf::Vector2f &frameSize,
                                   float scale)
{
    const float hpBarWidth = 120.f;
    const float hpBarHeight = 12.f;
    const sf::Vector2f barPos{boss.getPosition().x + (frameSize.x * scale - hpBarWidth) / 2.f,
                              boss.getPosition().y - 30.f};

    // Draw boss HP bar background
    window.draw(makeBar(barPos, {hpBarWidth, hpBarHeight}, sf::Color(60, 0, 0), sf::Color::Yellow, 2.f));

    // Draw current HP portion
    const float hpRatio = static_cast<float>(boss.getHp()) / static_cast<float>(boss.getMaxHp());
    window.draw(makeBar(barPos, {hpBarWidth * hpRatio, hpBarHeight}, sf::Color::Red, sf::Color::Transparent, 0.f));
}

// Render boss status effects and rage state
void UISystem::renderBossStatusEffects(sf::RenderWindow &window,
                                       const Boss &boss,
                                       bool bossRageMode)
{
    sf::FloatRect bounds = boss.getCollisionBounds();

    // Frozen or Slow indicator
    if (boss.getIceStacks() > 0)
    {
        sf::Vector2f indicatorPos(bounds.left + bounds.width / 2.f - 20.f, bounds.top - 50.f);
        std::string text = boss.isFrozen() ? "FROZEN" : "SLOW";
        sf::Text statusText(text, resourceManager.getFont(), 16);
        statusText.setPosition(indicatorPos);
        statusText.setFillColor(sf::Color::Cyan);
        statusText.setOutlineColor(sf::Color::Black);
        statusText.setOutlineThickness(2.f);
        window.draw(statusText);
    }

    // Burning indicator
    if (boss.getIsBurning())
    {
        sf::Vector2f indicatorPos(bounds.left + bounds.width / 2.f - 30.f, bounds.top - 90.f);
        sf::Text statusText("BURNING", resourceManager.getFont(), 16);
        statusText.setPosition(indicatorPos);
        statusText.setFillColor(sf::Color(255, 100, 0));
        statusText.setOutlineColor(sf::Color::Black);
        statusText.setOutlineThickness(2.f);
        window.draw(statusText);
    }

    // Rage indicator
    if (bossRageMode)
    {
        sf::Vector2f indicatorPos(bounds.left + bounds.width / 2.f - 40.f, bounds.top - 130.f);
        sf::Text statusText("ENRAGED!", resourceManager.getFont(), 20);
        statusText.setPosition(indicatorPos);
        statusText.setFillColor(sf::Color(255, 0, 0));
        statusText.setOutlineColor(sf::Color::Black);
        statusText.setOutlineThickness(2.5f);
        window.draw(statusText);
    }
}

// Utility: create a rectangle bar (generic UI element)
sf::RectangleShape UISystem::makeBar(sf::Vector2f position,
                                     sf::Vector2f size,
                                     const sf::Color &fill,
                                     const sf::Color &outline,
                                     float outlineThickness)
{
    sf::RectangleShape shape(size);
    shape.setPosition(position);
    shape.setFillColor(fill);
    shape.setOutlineColor(outline);
    shape.setOutlineThickness(outlineThickness);
    return shape;
}

// Utility: return a tinted copy of a sprite
sf::Sprite UISystem::makeTintedSprite(const sf::Sprite &sprite, const sf::Color &tint)
{
    sf::Sprite copy = sprite;
    copy.setColor(tint);
    return copy;
}

// Render a centered alert box with a message
void UISystem::renderCenteredAlert(sf::RenderWindow &window,
                                   const std::string &message,
                                   float yOffset,
                                   const sf::Color &textColor,
                                   const sf::Color &boxColor,
                                   unsigned int fontSize)
{
    // Create text
    sf::Text alertText(message, resourceManager.getFont(), fontSize);
    alertText.setFillColor(textColor);
    alertText.setOutlineColor(sf::Color::Black);
    alertText.setOutlineThickness(2.f);
    alertText.setStyle(sf::Text::Bold);

    // Center horizontally
    sf::FloatRect textBounds = alertText.getLocalBounds();
    float textX = (SCREEN_WIDTH - textBounds.width) / 2.f;
    alertText.setPosition(textX, yOffset);

    // Create background rectangle behind the text
    sf::RectangleShape alertBox(sf::Vector2f(textBounds.width + 20.f, textBounds.height + 20.f));
    alertBox.setPosition(textX - 10.f, yOffset - 5.f);
    alertBox.setFillColor(sf::Color(0, 0, 0, 180));
    alertBox.setOutlineColor(boxColor);
    alertBox.setOutlineThickness(3.f);

    // Draw elements
    window.draw(alertBox);
    window.draw(alertText);
}

// Draws the main menu (title + options + basic control hint)
void UISystem::renderMainMenu(sf::RenderWindow &window,
                              const GameStateData &mainMenuState)
{
    // Use a screen-space view (UI shouldn't be affected by gameplay camera)
    sf::View screenView(sf::FloatRect(0.f, 0.f, SCREEN_WIDTH, SCREEN_HEIGHT));
    window.setView(screenView);

    // Background fill
    sf::RectangleShape background(sf::Vector2f(SCREEN_WIDTH, SCREEN_HEIGHT));
    background.setFillColor(sf::Color(20, 20, 30));
    window.draw(background);

    // Shared font for all UI text
    const sf::Font &font = resourceManager.getFont();

    // Precompute the screen center once
    sf::Vector2f screenCenter(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f);

    // Game title
    sf::Text titleText;
    titleText.setFont(font);
    titleText.setString("Platformer Warrior");
    titleText.setCharacterSize(70);
    titleText.setFillColor(sf::Color::White);
    titleText.setOutlineColor(sf::Color::Black);
    titleText.setOutlineThickness(4.f);
    // Center origin around glyph bounds to align by center position
    sf::FloatRect titleBounds = titleText.getLocalBounds();
    titleText.setOrigin(titleBounds.width / 2.f, titleBounds.height / 2.f);
    titleText.setPosition(screenCenter.x, screenCenter.y - 200.f);
    window.draw(titleText);

    // Main menu options
    const char *menuOptions[] = {"New Game", "Load Game", "Exit"};
    const int numOptions = 3;

    // Draw each option with a highlight for the selected index
    for (int i = 0; i < numOptions; ++i)
    {
        sf::Text optionText;
        optionText.setFont(font);
        optionText.setString(menuOptions[i]);
        optionText.setCharacterSize(48);

        if (i == mainMenuState.selectedMenuOption)
        {
            // Selected state styling
            optionText.setFillColor(sf::Color::White);
            optionText.setOutlineColor(sf::Color::Black);
            optionText.setOutlineThickness(3.f);
            optionText.setCharacterSize(52);
        }
        else
        {
            // Unselected state styling
            optionText.setFillColor(sf::Color(150, 150, 150));
            optionText.setOutlineColor(sf::Color::Black);
            optionText.setOutlineThickness(2.f);
        }

        // Center each option text
        sf::FloatRect optionBounds = optionText.getLocalBounds();
        optionText.setOrigin(optionBounds.width / 2.f, optionBounds.height / 2.f);
        optionText.setPosition(screenCenter.x, screenCenter.y + i * 80.f);
        window.draw(optionText);
    }

    // Bottom control hint
    sf::Text controlsText;
    controlsText.setFont(font);
    controlsText.setString("UP/DOWN: Navigate  |  ENTER: Select");
    controlsText.setCharacterSize(20);
    controlsText.setFillColor(sf::Color(150, 150, 150));
    // Center on the bottom
    sf::FloatRect controlsBounds = controlsText.getLocalBounds();
    controlsText.setOrigin(controlsBounds.width / 2.f, controlsBounds.height / 2.f);
    controlsText.setPosition(screenCenter.x, SCREEN_HEIGHT - 50.f);
    window.draw(controlsText);
}

// Renders the appropriate UI menu based on the current game state
void UISystem::renderCurrentMenus(sf::RenderWindow &window,
                                  sf::View &camera,
                                  const Stack<GameStateData> &stateStack,
                                  Player *player,
                                  Boss *boss,
                                  SinglyLinkedList<std::string> &combatLog,
                                  int combatLogCurrentNode,
                                  int combatLogTraversalCount,
                                  int combatLogDeleteCount,
                                  const KeyBindingManager *keyManager)
{
    // If no states exist, nothing to render
    if (stateStack.isEmpty())
        return;

    // Get the top-most active state
    GameStateType currentState = stateStack.top().type;

    // Render pause menu
    if (currentState == GameStateType::Paused)
    {
        renderPauseMenu(window, stateStack.top());
        window.setView(camera); // Restore gameplay view
    }

    // Render player stats menu
    if (currentState == GameStateType::PlayerStatsScreen)
    {
        renderPlayerStatsScreen(window, player);
        window.setView(camera);
    }

    // Render inventory-only view
    if (currentState == GameStateType::InventoryOnlyScreen)
    {
        renderInventoryOnlyScreen(window, player);
        window.setView(camera);
    }

    // Render help/instructions menu
    if (currentState == GameStateType::HelpScreen)
    {
        renderHelpScreen(window);
        window.setView(camera);
    }

    // Render combat log screen
    if (currentState == GameStateType::CombatLogScreen)
    {
        renderCombatLog(window, combatLog, combatLogCurrentNode,
                        combatLogTraversalCount, combatLogDeleteCount);
        window.setView(camera);
    }

    // Render settings menu
    if (currentState == GameStateType::SettingsMenu)
    {
        renderSettingsMenu(window, stateStack.top());
        window.setView(camera);
    }

    // Render audio settings menu
    if (currentState == GameStateType::AudioSettings)
    {
        renderAudioSettings(window, stateStack.top());
        window.setView(camera);
    }

    // Render controls/keybind menu
    if (currentState == GameStateType::ControlsMenu && keyManager)
    {
        renderControlsMenu(window, stateStack.top(), *keyManager);
        window.setView(camera);
    }

    // Render confirmation dialogs (quit, restart, overwrite)
    if (currentState == GameStateType::ConfirmQuitToMenu ||
        currentState == GameStateType::ConfirmRestart ||
        currentState == GameStateType::ConfirmOverwriteSave)
    {
        renderConfirmDialog(window, stateStack.top());
        window.setView(camera);
    }
}

// Pause menu overlay with options and hint
void UISystem::renderPauseMenu(sf::RenderWindow &window,
                               const GameStateData &pauseState)
{
    // UI view
    sf::View screenView(sf::FloatRect(0.f, 0.f, SCREEN_WIDTH, SCREEN_HEIGHT));
    window.setView(screenView);

    // Dark overlay
    sf::RectangleShape overlay(sf::Vector2f(SCREEN_WIDTH, SCREEN_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 210));
    overlay.setPosition(0.f, 0.f);
    window.draw(overlay);

    // Font and center
    const sf::Font &font = resourceManager.getFont();
    sf::Vector2f screenCenter(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f);

    // Title
    sf::Text titleText;
    titleText.setFont(font);
    titleText.setString("PAUSED");
    titleText.setCharacterSize(60);
    titleText.setFillColor(sf::Color::White);
    titleText.setOutlineColor(sf::Color::Black);
    titleText.setOutlineThickness(3.f);
    sf::FloatRect titleBounds = titleText.getLocalBounds();
    titleText.setOrigin(titleBounds.width / 2.f, titleBounds.height / 2.f);
    titleText.setPosition(screenCenter.x, screenCenter.y - 150.f);
    window.draw(titleText);

    // Options for pause menu
    const char *menuOptions[] = {"Resume", "Save Game", "Combat Log", "Settings", "Restart", "Quit to Menu"};
    const int numOptions = 6;

    // Draw list of options
    for (int i = 0; i < numOptions; ++i)
    {
        sf::Text optionText;
        optionText.setFont(font);
        optionText.setString(menuOptions[i]);
        optionText.setCharacterSize(32);

        if (i == pauseState.selectedMenuOption)
        {
            // Selected
            optionText.setFillColor(sf::Color::White); // Selected: white
            optionText.setOutlineColor(sf::Color::Black);
            optionText.setOutlineThickness(2.f);
            optionText.setCharacterSize(36);
        }
        else
        {
            // Unselected
            optionText.setFillColor(sf::Color(150, 150, 150)); // Not selected: gray
            optionText.setOutlineColor(sf::Color::Black);
            optionText.setOutlineThickness(1.5f);
        }

        // Center each row
        sf::FloatRect optionBounds = optionText.getLocalBounds();
        optionText.setOrigin(optionBounds.width / 2.f, optionBounds.height / 2.f);
        optionText.setPosition(screenCenter.x, screenCenter.y - 40.f + i * 60.f);
        window.draw(optionText);
    }

    // Bottom hint
    sf::Text controlsText;
    controlsText.setFont(font);
    controlsText.setString("UP/DOWN: Navigate  |  ENTER: Select  |  ESC: Resume");
    controlsText.setCharacterSize(18);
    controlsText.setFillColor(sf::Color(180, 180, 180));
    sf::FloatRect controlsBounds = controlsText.getLocalBounds();
    controlsText.setOrigin(controlsBounds.width / 2.f, controlsBounds.height / 2.f);
    controlsText.setPosition(screenCenter.x, screenCenter.y + 350.f); // Moved further down
    window.draw(controlsText);
}

// Screen-filling flashing warning when HP is low
void UISystem::renderLowHpWarning(sf::RenderWindow &window,
                                  float lowHpWarningTimer,
                                  const Player *player)
{
    // Player required
    if (!player)
        return;

    // HP fraction from 0..1
    float hpPercent = static_cast<float>(player->getHp()) / static_cast<float>(player->getMaxHp());
    constexpr float LOW_HP_THRESHOLD = 0.40f;  // Trigger threshold
    constexpr float LOW_HP_FLASH_SPEED = 1.5f; // Flash speed multiplier

    // If above threshold, nothing to draw
    if (hpPercent > LOW_HP_THRESHOLD)
        return;

    // Sinusoidal alpha modulation for flashing effect
    float flashCycle = lowHpWarningTimer * LOW_HP_FLASH_SPEED * 2.0f * 3.14159f;
    float flashValue = (std::sin(flashCycle) + 1.0f) / 2.0f;

    // Make flash stronger as HP gets lower
    float intensityMultiplier = 1.0f - (hpPercent / LOW_HP_THRESHOLD);
    sf::Uint8 alpha = static_cast<sf::Uint8>(flashValue * 80.0f * intensityMultiplier);

    // Use UI view
    sf::View screenView(sf::FloatRect(0.f, 0.f, SCREEN_WIDTH, SCREEN_HEIGHT));
    window.setView(screenView);

    // Red overlay with animated alpha
    sf::RectangleShape overlay(sf::Vector2f(SCREEN_WIDTH, SCREEN_HEIGHT));
    overlay.setFillColor(sf::Color(255, 0, 0, alpha));
    overlay.setPosition(0.f, 0.f);
    window.draw(overlay);
}

// Inventory + stats popup that pauses the game
void UISystem::renderInventoryScreen(sf::RenderWindow &window,
                                     const Player *player)
{
    if (!player)
        return;

    // UI view
    sf::View screenView(sf::FloatRect(0.f, 0.f, SCREEN_WIDTH, SCREEN_HEIGHT));
    window.setView(screenView);

    // Dimmed overlay
    sf::RectangleShape overlay(sf::Vector2f(SCREEN_WIDTH, SCREEN_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 210));
    overlay.setPosition(0.f, 0.f);
    window.draw(overlay);

    // Font + center
    const sf::Font &font = resourceManager.getFont();
    sf::Vector2f screenCenter(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f);

    const int hpPotions = player->getHpPotions();
    const int maxHpPotions = player->getMaxHpPotions();

    // Title
    sf::Text titleText;
    titleText.setFont(font);
    titleText.setString("INVENTORY & STATS");
    titleText.setCharacterSize(48);
    titleText.setFillColor(sf::Color(100, 200, 255));
    titleText.setOutlineColor(sf::Color::Black);
    titleText.setOutlineThickness(3.f);
    sf::FloatRect titleBounds = titleText.getLocalBounds();
    titleText.setOrigin(titleBounds.width / 2.f, titleBounds.height / 2.f);
    titleText.setPosition(screenCenter.x, screenCenter.y - 250.f);
    window.draw(titleText);

    // Left column: Player stats header
    sf::Text sectionTitle;
    sectionTitle.setFont(font);
    sectionTitle.setString("PLAYER STATS");
    sectionTitle.setCharacterSize(32);
    sectionTitle.setFillColor(sf::Color(255, 255, 100));
    sectionTitle.setOutlineColor(sf::Color::Black);
    sectionTitle.setOutlineThickness(2.f);
    sf::FloatRect sectionBounds = sectionTitle.getLocalBounds();
    sectionTitle.setOrigin(sectionBounds.width / 2.f, sectionBounds.height / 2.f);
    sectionTitle.setPosition(screenCenter.x - 250.f, screenCenter.y - 180.f);
    window.draw(sectionTitle);

    // Build stats text block
    std::string statsText =
        "Level: " + std::to_string(player->getLevel()) + "\n\n" +
        "HP: " + std::to_string(player->getHp()) + " / " + std::to_string(player->getMaxHp()) + "\n\n" +
        "Damage: " + std::to_string(player->getDamage()) + "\n\n" +
        "Experience: " + std::to_string(player->getExperience()) + " / " + std::to_string(player->getExpToNextLevel());

    // Stats text rendering
    sf::Text statsDisplay;
    statsDisplay.setFont(font);
    statsDisplay.setString(statsText);
    statsDisplay.setCharacterSize(24);
    statsDisplay.setFillColor(sf::Color::White);
    statsDisplay.setOutlineColor(sf::Color::Black);
    statsDisplay.setOutlineThickness(1.5f);
    statsDisplay.setLineSpacing(1.2f);
    statsDisplay.setPosition(screenCenter.x - 350.f, screenCenter.y - 120.f);
    window.draw(statsDisplay);

    // HP bar (left column)
    float hpPercent = static_cast<float>(player->getHp()) / static_cast<float>(player->getMaxHp());
    sf::RectangleShape hpBarBg(sf::Vector2f(200.f, 20.f));
    hpBarBg.setPosition(screenCenter.x - 350.f, screenCenter.y - 45.f);
    hpBarBg.setFillColor(sf::Color(50, 50, 50));
    hpBarBg.setOutlineColor(sf::Color::White);
    hpBarBg.setOutlineThickness(2.f);
    window.draw(hpBarBg);

    sf::RectangleShape hpBarFill(sf::Vector2f(196.f * hpPercent, 16.f));
    hpBarFill.setPosition(screenCenter.x - 348.f, screenCenter.y - 43.f);
    // Color by thresholds
    if (hpPercent > 0.6f)
        hpBarFill.setFillColor(sf::Color(50, 200, 50));
    else if (hpPercent > 0.3f)
        hpBarFill.setFillColor(sf::Color(255, 200, 50));
    else
        hpBarFill.setFillColor(sf::Color(200, 50, 50));
    window.draw(hpBarFill);

    // EXP bar (left column)
    float expPercent = static_cast<float>(player->getExperience()) / static_cast<float>(player->getExpToNextLevel());
    sf::RectangleShape expBarBg(sf::Vector2f(200.f, 15.f));
    expBarBg.setPosition(screenCenter.x - 350.f, screenCenter.y + 65.f);
    expBarBg.setFillColor(sf::Color(30, 30, 50));
    expBarBg.setOutlineColor(sf::Color(100, 100, 150));
    expBarBg.setOutlineThickness(2.f);
    window.draw(expBarBg);

    sf::RectangleShape expBarFill(sf::Vector2f(196.f * expPercent, 11.f));
    expBarFill.setPosition(screenCenter.x - 348.f, screenCenter.y + 67.f);
    expBarFill.setFillColor(sf::Color(100, 150, 255));
    window.draw(expBarFill);

    // Right column: Inventory header
    sf::Text inventoryTitle;
    inventoryTitle.setFont(font);
    inventoryTitle.setString("INVENTORY");
    inventoryTitle.setCharacterSize(32);
    inventoryTitle.setFillColor(sf::Color(100, 255, 100));
    inventoryTitle.setOutlineColor(sf::Color::Black);
    inventoryTitle.setOutlineThickness(2.f);
    sf::FloatRect invTitleBounds = inventoryTitle.getLocalBounds();
    inventoryTitle.setOrigin(invTitleBounds.width / 2.f, invTitleBounds.height / 2.f);
    inventoryTitle.setPosition(screenCenter.x + 200.f, screenCenter.y - 180.f);
    window.draw(inventoryTitle);

    // Potion label
    sf::Text potionLabel;
    potionLabel.setFont(font);
    potionLabel.setString("HP Potions:");
    potionLabel.setCharacterSize(24);
    potionLabel.setFillColor(sf::Color(200, 200, 200));
    potionLabel.setOutlineColor(sf::Color::Black);
    potionLabel.setOutlineThickness(1.5f);
    potionLabel.setPosition(screenCenter.x + 80.f, screenCenter.y - 120.f);
    window.draw(potionLabel);

    // Potion count, large font value
    sf::Text potionCount;
    potionCount.setFont(font);
    potionCount.setString(std::to_string(hpPotions) + " / " + std::to_string(maxHpPotions));
    potionCount.setCharacterSize(36);
    potionCount.setFillColor(sf::Color(255, 100, 200));
    potionCount.setOutlineColor(sf::Color::Black);
    potionCount.setOutlineThickness(2.f);
    potionCount.setPosition(screenCenter.x + 80.f, screenCenter.y - 85.f);
    window.draw(potionCount);

    // Visual slots for each potion capacity
    for (int i = 0; i < maxHpPotions; ++i)
    {
        sf::RectangleShape potionSlot(sf::Vector2f(30.f, 40.f));
        potionSlot.setPosition(screenCenter.x + 80.f + i * 40.f, screenCenter.y - 30.f);

        if (i < hpPotions)
        {
            // Filled slot (owned)
            potionSlot.setFillColor(sf::Color(255, 100, 200));
            potionSlot.setOutlineColor(sf::Color::White);
        }
        else
        {
            // Empty slot (capacity)
            potionSlot.setFillColor(sf::Color(50, 50, 50, 150));
            potionSlot.setOutlineColor(sf::Color(100, 100, 100));
        }
        potionSlot.setOutlineThickness(2.f);
        window.draw(potionSlot);
    }

    // Hint: using potions
    sf::Text potionHint;
    potionHint.setFont(font);
    potionHint.setString("Press Q to use potion (+30 HP)");
    potionHint.setCharacterSize(18);
    potionHint.setFillColor(sf::Color(200, 200, 200));
    potionHint.setPosition(screenCenter.x + 80.f, screenCenter.y + 20.f);
    window.draw(potionHint);

    // Controls section header
    sf::Text controlsTitle;
    controlsTitle.setFont(font);
    controlsTitle.setString("CONTROLS");
    controlsTitle.setCharacterSize(28);
    controlsTitle.setFillColor(sf::Color(255, 200, 100));
    controlsTitle.setOutlineColor(sf::Color::Black);
    controlsTitle.setOutlineThickness(2.f);
    controlsTitle.setPosition(screenCenter.x + 80.f, screenCenter.y + 80.f);
    window.draw(controlsTitle);

    // Controls list (right column)
    std::string controlsText =
        "Movement: Arrow Keys\n"
        "Jump: SPACE (double jump)\n"
        "Attack: A (normal), S (special)\n"
        "Fireball: F\n"
        "Use Potion: Q\n"
        "Pause: ESC";

    sf::Text controlsDisplay;
    controlsDisplay.setFont(font);
    controlsDisplay.setString(controlsText);
    controlsDisplay.setCharacterSize(18);
    controlsDisplay.setFillColor(sf::Color(220, 220, 220));
    controlsDisplay.setOutlineColor(sf::Color::Black);
    controlsDisplay.setOutlineThickness(1.f);
    controlsDisplay.setLineSpacing(1.3f);
    controlsDisplay.setPosition(screenCenter.x + 80.f, screenCenter.y + 120.f);
    window.draw(controlsDisplay);

    // Close hint
    sf::Text closeHint;
    closeHint.setFont(font);
    closeHint.setString("Press I or ESC to close");
    closeHint.setCharacterSize(20);
    closeHint.setFillColor(sf::Color(180, 180, 180));
    sf::FloatRect hintBounds = closeHint.getLocalBounds();
    closeHint.setOrigin(hintBounds.width / 2.f, hintBounds.height / 2.f);
    closeHint.setPosition(screenCenter.x, screenCenter.y + 280.f);
    window.draw(closeHint);
}

// Player stats screen (no inventory)
void UISystem::renderPlayerStatsScreen(sf::RenderWindow &window,
                                       const Player *player)
{
    if (!player)
        return;

    // UI view
    sf::View screenView(sf::FloatRect(0.f, 0.f, SCREEN_WIDTH, SCREEN_HEIGHT));
    window.setView(screenView);

    // Dimmed overlay
    sf::RectangleShape overlay(sf::Vector2f(SCREEN_WIDTH, SCREEN_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 210));
    overlay.setPosition(0.f, 0.f);
    window.draw(overlay);

    // Font + center
    const sf::Font &font = resourceManager.getFont();
    sf::Vector2f screenCenter(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f);

    // Title
    sf::Text titleText;
    titleText.setFont(font);
    titleText.setString("PLAYER STATS");
    titleText.setCharacterSize(48);
    titleText.setFillColor(sf::Color(255, 255, 100));
    titleText.setOutlineColor(sf::Color::Black);
    titleText.setOutlineThickness(3.f);
    sf::FloatRect titleBounds = titleText.getLocalBounds();
    titleText.setOrigin(titleBounds.width / 2.f, titleBounds.height / 2.f);
    titleText.setPosition(screenCenter.x, screenCenter.y - 200.f);
    window.draw(titleText);

    // Stats block
    std::string statsText =
        "Level: " + std::to_string(player->getLevel()) + "\n\n" +
        "HP: " + std::to_string(player->getHp()) + " / " + std::to_string(player->getMaxHp()) + "\n\n" +
        "Damage: " + std::to_string(player->getDamage()) + "\n\n" +
        "Experience: " + std::to_string(player->getExperience()) + " / " + std::to_string(player->getExpToNextLevel());

    sf::Text statsDisplay;
    statsDisplay.setFont(font);
    statsDisplay.setString(statsText);
    statsDisplay.setCharacterSize(28);
    statsDisplay.setFillColor(sf::Color::White);
    statsDisplay.setOutlineColor(sf::Color::Black);
    statsDisplay.setOutlineThickness(1.5f);
    statsDisplay.setLineSpacing(1.3f);

    // Center block
    sf::FloatRect statsBounds = statsDisplay.getLocalBounds();
    statsDisplay.setOrigin(statsBounds.width / 2.f, 0.f);
    statsDisplay.setPosition(screenCenter.x, screenCenter.y - 120.f);
    window.draw(statsDisplay);

    // HP bar
    float hpPercent = static_cast<float>(player->getHp()) / static_cast<float>(player->getMaxHp());
    sf::RectangleShape hpBarBg(sf::Vector2f(300.f, 25.f));
    hpBarBg.setPosition(screenCenter.x - 150.f, screenCenter.y + 40.f);
    hpBarBg.setFillColor(sf::Color(50, 50, 50));
    hpBarBg.setOutlineColor(sf::Color::White);
    hpBarBg.setOutlineThickness(2.f);
    window.draw(hpBarBg);

    sf::RectangleShape hpBarFill(sf::Vector2f(296.f * hpPercent, 21.f));
    hpBarFill.setPosition(screenCenter.x - 148.f, screenCenter.y + 42.f);
    if (hpPercent > 0.6f)
        hpBarFill.setFillColor(sf::Color(50, 200, 50));
    else if (hpPercent > 0.3f)
        hpBarFill.setFillColor(sf::Color(255, 200, 50));
    else
        hpBarFill.setFillColor(sf::Color(200, 50, 50));
    window.draw(hpBarFill);

    // EXP bar
    float expPercent = static_cast<float>(player->getExperience()) / static_cast<float>(player->getExpToNextLevel());
    sf::RectangleShape expBarBg(sf::Vector2f(300.f, 20.f));
    expBarBg.setPosition(screenCenter.x - 150.f, screenCenter.y + 165.f);
    expBarBg.setFillColor(sf::Color(30, 30, 50));
    expBarBg.setOutlineColor(sf::Color(100, 100, 150));
    expBarBg.setOutlineThickness(2.f);
    window.draw(expBarBg);

    sf::RectangleShape expBarFill(sf::Vector2f(296.f * expPercent, 16.f));
    expBarFill.setPosition(screenCenter.x - 148.f, screenCenter.y + 167.f);
    expBarFill.setFillColor(sf::Color(100, 150, 255));
    window.draw(expBarFill);

    // Exit hint
    sf::Text closeHint;
    closeHint.setFont(font);
    closeHint.setString("Press ESC to go back");
    closeHint.setCharacterSize(20);
    closeHint.setFillColor(sf::Color(180, 180, 180));
    sf::FloatRect hintBounds2 = closeHint.getLocalBounds();
    closeHint.setOrigin(hintBounds2.width / 2.f, hintBounds2.height / 2.f);
    closeHint.setPosition(screenCenter.x, screenCenter.y + 250.f);
    window.draw(closeHint);
}

// Inventory-only overlay (no stats column)
void UISystem::renderInventoryOnlyScreen(sf::RenderWindow &window,
                                         const Player *player)
{
    if (!player)
        return;

    // UI view
    sf::View screenView(sf::FloatRect(0.f, 0.f, SCREEN_WIDTH, SCREEN_HEIGHT));
    window.setView(screenView);

    // Dim overlay
    sf::RectangleShape overlay(sf::Vector2f(SCREEN_WIDTH, SCREEN_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 210));
    overlay.setPosition(0.f, 0.f);
    window.draw(overlay);

    // Font + center
    const sf::Font &font = resourceManager.getFont();
    sf::Vector2f screenCenter(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f);

    const int hpPotions = player->getHpPotions();
    const int maxHpPotions = player->getMaxHpPotions();

    // Title
    sf::Text titleText;
    titleText.setFont(font);
    titleText.setString("INVENTORY");
    titleText.setCharacterSize(48);
    titleText.setFillColor(sf::Color(100, 255, 100));
    titleText.setOutlineColor(sf::Color::Black);
    titleText.setOutlineThickness(3.f);
    sf::FloatRect titleBounds = titleText.getLocalBounds();
    titleText.setOrigin(titleBounds.width / 2.f, titleBounds.height / 2.f);
    titleText.setPosition(screenCenter.x, screenCenter.y - 180.f);
    window.draw(titleText);

    // Potions label centered
    sf::Text potionLabel;
    potionLabel.setFont(font);
    potionLabel.setString("HP Potions:");
    potionLabel.setCharacterSize(28);
    potionLabel.setFillColor(sf::Color(200, 200, 200));
    potionLabel.setOutlineColor(sf::Color::Black);
    potionLabel.setOutlineThickness(1.5f);
    sf::FloatRect labelBounds = potionLabel.getLocalBounds();
    potionLabel.setOrigin(labelBounds.width / 2.f, labelBounds.height / 2.f);
    potionLabel.setPosition(screenCenter.x, screenCenter.y - 80.f);
    window.draw(potionLabel);

    // Count text
    sf::Text potionCount;
    potionCount.setFont(font);
    potionCount.setString(std::to_string(hpPotions) + " / " + std::to_string(maxHpPotions));
    potionCount.setCharacterSize(40);
    potionCount.setFillColor(sf::Color(255, 100, 200));
    potionCount.setOutlineColor(sf::Color::Black);
    potionCount.setOutlineThickness(2.f);
    sf::FloatRect countBounds = potionCount.getLocalBounds();
    potionCount.setOrigin(countBounds.width / 2.f, countBounds.height / 2.f);
    potionCount.setPosition(screenCenter.x, screenCenter.y - 30.f);
    window.draw(potionCount);

    // Slot row centered beneath count
    float startX = screenCenter.x - (maxHpPotions * 40.f / 2.f);
    for (int i = 0; i < maxHpPotions; ++i)
    {
        sf::RectangleShape potionSlot(sf::Vector2f(30.f, 40.f));
        potionSlot.setPosition(startX + i * 40.f, screenCenter.y + 30.f);

        if (i < hpPotions)
        {
            // Filled slot
            potionSlot.setFillColor(sf::Color(255, 100, 200));
            potionSlot.setOutlineColor(sf::Color::White);
        }
        else
        {
            // Empty slot
            potionSlot.setFillColor(sf::Color(50, 50, 50, 150));
            potionSlot.setOutlineColor(sf::Color(100, 100, 100));
        }
        potionSlot.setOutlineThickness(2.f);
        window.draw(potionSlot);
    }

    // Usage hint
    sf::Text potionHint;
    potionHint.setFont(font);
    potionHint.setString("Press Q during gameplay to use potion (+30 HP)");
    potionHint.setCharacterSize(20);
    potionHint.setFillColor(sf::Color(200, 200, 200));
    sf::FloatRect potionHintBounds = potionHint.getLocalBounds();
    potionHint.setOrigin(potionHintBounds.width / 2.f, potionHintBounds.height / 2.f);
    potionHint.setPosition(screenCenter.x, screenCenter.y + 100.f);
    window.draw(potionHint);

    // Exit hint
    sf::Text closeHint;
    closeHint.setFont(font);
    closeHint.setString("Press ESC to go back");
    closeHint.setCharacterSize(20);
    closeHint.setFillColor(sf::Color(180, 180, 180));
    sf::FloatRect hintBounds = closeHint.getLocalBounds();
    closeHint.setOrigin(hintBounds.width / 2.f, hintBounds.height / 2.f);
    closeHint.setPosition(screenCenter.x, screenCenter.y + 220.f);
    window.draw(closeHint);
}

// Skill tree screen: shows nodes, connections, and bonuses
void UISystem::renderSkillTreeScreen(sf::RenderWindow &window,
                                     const Player *player,
                                     int selectedSkillIndex)
{
    if (!player)
        return;

    // UI view
    sf::View screenView(sf::FloatRect(0.f, 0.f, SCREEN_WIDTH, SCREEN_HEIGHT));
    window.setView(screenView);

    // Dim overlay
    sf::RectangleShape overlay(sf::Vector2f(SCREEN_WIDTH, SCREEN_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 220));
    overlay.setPosition(0.f, 0.f);
    window.draw(overlay);

    // Font + center
    const sf::Font &font = resourceManager.getFont();
    sf::Vector2f screenCenter(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f);

    // Skill tree data from player
    const SkillTree &skillTree = player->getSkillTree();
    const auto *specialAttackNode = skillTree.getSkillNode(SkillType::SPECIAL_ATTACK);
    const bool specialAttackUnlocked = specialAttackNode && !specialAttackNode->isEmpty() && specialAttackNode->getKey().isUnlocked();
    const int specialAttackLevel = specialAttackNode ? specialAttackNode->getKey().currentLevel : 0;
    const int specialAttackMaxLevel = specialAttackNode ? specialAttackNode->getKey().maxLevel : 1;

    // Title
    sf::Text titleText;
    titleText.setFont(font);
    titleText.setString("SKILL TREE");
    titleText.setCharacterSize(48);
    titleText.setFillColor(sf::Color(255, 215, 0));
    titleText.setOutlineColor(sf::Color::Black);
    titleText.setOutlineThickness(3.f);
    sf::FloatRect titleBounds = titleText.getLocalBounds();
    titleText.setOrigin(titleBounds.width / 2.f, titleBounds.height / 2.f);
    titleText.setPosition(screenCenter.x, 60.f);
    window.draw(titleText);

    // Skill points display
    sf::Text skillPointsText;
    skillPointsText.setFont(font);
    skillPointsText.setString("Available Skill Points: " + std::to_string(skillTree.getSkillPoints()));
    skillPointsText.setCharacterSize(28);
    skillPointsText.setFillColor(sf::Color(100, 255, 100));
    skillPointsText.setOutlineColor(sf::Color::Black);
    skillPointsText.setOutlineThickness(2.f);
    sf::FloatRect spBounds = skillPointsText.getLocalBounds();
    skillPointsText.setOrigin(spBounds.width / 2.f, spBounds.height / 2.f);
    skillPointsText.setPosition(screenCenter.x, 120.f);
    window.draw(skillPointsText);

    // Layout positions for tiers and branches
    float rootY = 220.f;
    float tier1Y = 310.f;
    float tier2Y = 420.f;
    float leftX = screenCenter.x - 250.f;
    float rightX = screenCenter.x + 250.f;

    // Helper to draw a skill node circle + labels and optional selection highlight
    auto drawSkillNode = [&](const std::string &name, bool unlocked, int level, int maxLevel,
                             float x, float y, sf::Color nodeColor, bool isSelected)
    {
        // Selected highlight ring
        if (isSelected)
        {
            sf::CircleShape selectionHighlight(45.f);
            selectionHighlight.setPosition(x - 45.f, y - 45.f);
            selectionHighlight.setFillColor(sf::Color::Transparent);
            selectionHighlight.setOutlineColor(sf::Color::Yellow);
            selectionHighlight.setOutlineThickness(4.f);
            window.draw(selectionHighlight);
        }

        // Node icon
        sf::CircleShape node(35.f);
        node.setPosition(x - 35.f, y - 35.f);

        if (unlocked)
        {
            node.setFillColor(nodeColor);
            node.setOutlineColor(sf::Color(255, 255, 100));
            node.setOutlineThickness(3.f);
        }
        else
        {
            node.setFillColor(sf::Color(50, 50, 50));
            node.setOutlineColor(sf::Color(100, 100, 100));
            node.setOutlineThickness(2.f);
        }
        window.draw(node);

        // Name under the node
        sf::Text nameText;
        nameText.setFont(font);
        nameText.setString(name);
        nameText.setCharacterSize(16);
        nameText.setFillColor(unlocked ? sf::Color::White : sf::Color(120, 120, 120));
        nameText.setOutlineColor(sf::Color::Black);
        nameText.setOutlineThickness(1.f);
        sf::FloatRect nameBounds = nameText.getLocalBounds();
        nameText.setOrigin(nameBounds.width / 2.f, nameBounds.height / 2.f);
        nameText.setPosition(x, y + 62.f);
        window.draw(nameText);

        // Level text for multi-level nodes or a checkmark-like indicator (empty string in current design)
        if (maxLevel > 1)
        {
            sf::Text levelText;
            levelText.setFont(font);
            levelText.setString(std::to_string(level) + "/" + std::to_string(maxLevel));
            levelText.setCharacterSize(18);
            levelText.setFillColor(sf::Color::White);
            levelText.setOutlineColor(sf::Color::Black);
            levelText.setOutlineThickness(1.5f);
            sf::FloatRect lvlBounds = levelText.getLocalBounds();
            levelText.setOrigin(lvlBounds.width / 2.f, lvlBounds.height / 2.f);
            levelText.setPosition(x, y);
            window.draw(levelText);
        }
        else if (unlocked)
        {
            // For single-level unlocked nodes, draw placeholder text (no icon currently)
            sf::Text checkText;
            checkText.setFont(font);
            checkText.setString("");
            checkText.setCharacterSize(30);
            checkText.setFillColor(sf::Color::White);
            checkText.setOutlineColor(sf::Color::Black);
            checkText.setOutlineThickness(2.f);
            sf::FloatRect checkBounds = checkText.getLocalBounds();
            checkText.setOrigin(checkBounds.width / 2.f, checkBounds.height / 2.f);
            checkText.setPosition(x, y);
            window.draw(checkText);
        }
    };

    // Helper to draw a connection line between two nodes; color indicates active chain
    auto drawConnection = [&](float x1, float y1, float x2, float y2, bool active)
    {
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(x1, y1), active ? sf::Color(150, 150, 255) : sf::Color(80, 80, 80)),
            sf::Vertex(sf::Vector2f(x2, y2), active ? sf::Color(150, 150, 255) : sf::Color(80, 80, 80))};
        window.draw(line, 2, sf::Lines);
    };

    // Root node (Special attack)
    drawSkillNode("Special\nAttack (S)",
                  specialAttackUnlocked,
                  specialAttackLevel,
                  specialAttackMaxLevel,
                  screenCenter.x,
                  rootY,
                  sf::Color(200, 100, 200),
                  selectedSkillIndex == 0);

    // Branch connections to tier 1
    drawConnection(screenCenter.x - 30.f, rootY + 20.f, leftX, tier1Y - 25.f, skillTree.hasFireProjectile());
    drawConnection(screenCenter.x + 30.f, rootY + 20.f, rightX, tier1Y - 25.f, skillTree.hasIceProjectile());

    // Fire projectile node (tier 1)
    drawSkillNode("", skillTree.hasFireProjectile(),
                  skillTree.hasFireProjectile() ? 1 : 0, 1, leftX, tier1Y, sf::Color(255, 100, 50), selectedSkillIndex == 1);

    // Label for fire projectile
    sf::Text fireProjectileText;
    fireProjectileText.setFont(font);
    fireProjectileText.setString("Fire\nProjectile (F)");
    fireProjectileText.setCharacterSize(14);
    fireProjectileText.setFillColor(skillTree.hasFireProjectile() ? sf::Color::White : sf::Color(120, 120, 120));
    fireProjectileText.setOutlineColor(sf::Color::Black);
    fireProjectileText.setOutlineThickness(1.f);
    sf::FloatRect fireProjBounds = fireProjectileText.getLocalBounds();
    fireProjectileText.setOrigin(fireProjBounds.width / 2.f, fireProjBounds.height / 2.f);
    fireProjectileText.setPosition(leftX - 95.f, tier1Y);
    window.draw(fireProjectileText);

    // Ice projectile node (tier 1)
    drawSkillNode("", skillTree.hasIceProjectile(),
                  skillTree.hasIceProjectile() ? 1 : 0, 1, rightX, tier1Y, sf::Color(100, 200, 255), selectedSkillIndex == 2);

    // Label for ice projectile
    sf::Text iceProjectileText;
    iceProjectileText.setFont(font);
    iceProjectileText.setString("Ice\nProjectile (F)");
    iceProjectileText.setCharacterSize(14);
    iceProjectileText.setFillColor(skillTree.hasIceProjectile() ? sf::Color::White : sf::Color(120, 120, 120));
    iceProjectileText.setOutlineColor(sf::Color::Black);
    iceProjectileText.setOutlineThickness(1.f);
    sf::FloatRect iceProjBounds = iceProjectileText.getLocalBounds();
    iceProjectileText.setOrigin(iceProjBounds.width / 2.f, iceProjBounds.height / 2.f);
    iceProjectileText.setPosition(rightX + 95.f, tier1Y);
    window.draw(iceProjectileText);

    // Tier 2 connections from each branch
    bool fireUnlocked = skillTree.hasFireProjectile();
    drawConnection(leftX - 30.f, tier1Y + 20.f, leftX - 130.f, tier2Y - 35.f, fireUnlocked);
    drawConnection(leftX, tier1Y + 35.f, leftX, tier2Y - 35.f, fireUnlocked);
    drawConnection(leftX + 30.f, tier1Y + 20.f, leftX + 130.f, tier2Y - 35.f, fireUnlocked);

    bool iceUnlocked = skillTree.hasIceProjectile();
    drawConnection(rightX - 30.f, tier1Y + 20.f, rightX - 130.f, tier2Y - 35.f, iceUnlocked);
    drawConnection(rightX, tier1Y + 35.f, rightX, tier2Y - 35.f, iceUnlocked);
    drawConnection(rightX + 30.f, tier1Y + 20.f, rightX + 130.f, tier2Y - 35.f, iceUnlocked);

    // Tier 2 fire nodes (levels shown via level/maxLevel)
    int fireDmg = skillTree.getTotalFireDamage() / 5;
    drawSkillNode("Fire\nMastery", fireDmg > 0, fireDmg, 3, leftX - 130.f, tier2Y, sf::Color(255, 150, 0), selectedSkillIndex == 3);

    int fireCharges = std::max(0, skillTree.getFireProjectileCharges() - 1);
    drawSkillNode("Fire\nReserves", fireCharges > 0, fireCharges, 2, leftX, tier2Y, sf::Color(255, 100, 50), selectedSkillIndex == 4);

    int fireCount = std::max(0, skillTree.getFireProjectileCount() - 1);
    drawSkillNode("Fire\nBarrage", fireCount > 0, fireCount, 2, leftX + 130.f, tier2Y, sf::Color(255, 50, 0), selectedSkillIndex == 5);

    // Tier 2 ice nodes
    int iceDmg = skillTree.getTotalIceDamage() / 5;
    drawSkillNode("Ice\nMastery", iceDmg > 0, iceDmg, 3, rightX - 130.f, tier2Y, sf::Color(150, 220, 255), selectedSkillIndex == 6);

    int iceCharges = std::max(0, skillTree.getIceProjectileCharges() - 1);
    drawSkillNode("Ice\nReserves", iceCharges > 0, iceCharges, 2, rightX, tier2Y, sf::Color(100, 200, 255), selectedSkillIndex == 7);

    int iceCount = std::max(0, skillTree.getIceProjectileCount() - 1);
    drawSkillNode("Ice\nStorm", iceCount > 0, iceCount, 2, rightX + 130.f, tier2Y, sf::Color(100, 150, 255), selectedSkillIndex == 8);

    // Aggregate bonuses panel
    sf::Text bonusText;
    bonusText.setFont(font);
    std::string bonusInfo = "Active Bonuses:\n";
    if (skillTree.hasFireProjectile())
    {
        bonusInfo += "Fire: Dmg +" + std::to_string(skillTree.getTotalFireDamage()) +
                     " | Charges: " + std::to_string(skillTree.getFireProjectileCharges()) +
                     " | Projectiles: " + std::to_string(skillTree.getFireProjectileCount()) + "\n";
    }
    if (skillTree.hasIceProjectile())
    {
        bonusInfo += "Ice: Dmg +" + std::to_string(skillTree.getTotalIceDamage()) +
                     " | Charges: " + std::to_string(skillTree.getIceProjectileCharges()) +
                     " | Projectiles: " + std::to_string(skillTree.getIceProjectileCount()) + "\n";
    }
    if (!skillTree.hasFireProjectile() && !skillTree.hasIceProjectile())
    {
        bonusInfo += "None (unlock projectiles to gain bonuses)";
    }
    bonusText.setString(bonusInfo);
    bonusText.setCharacterSize(18);
    bonusText.setFillColor(sf::Color(200, 200, 255));
    bonusText.setOutlineColor(sf::Color::Black);
    bonusText.setOutlineThickness(1.f);
    sf::FloatRect bonusBounds = bonusText.getLocalBounds();
    bonusText.setOrigin(bonusBounds.width / 2.f, 0.f);
    bonusText.setPosition(screenCenter.x, tier2Y + 110.f);
    window.draw(bonusText);

    // Reset hint (shows possible branches to reset)
    sf::Text resetHint;
    resetHint.setFont(font);
    std::string resetText = "Reset Branch: ";
    if (skillTree.hasFireProjectile())
        resetText += "1 - Fire  ";
    if (skillTree.hasIceProjectile())
        resetText += "2 - Ice";
    if (!skillTree.hasFireProjectile() && !skillTree.hasIceProjectile())
        resetText += "(Unlock branches first)";
    resetHint.setString(resetText);
    resetHint.setCharacterSize(18);
    resetHint.setFillColor(sf::Color(255, 200, 100));
    resetHint.setOutlineColor(sf::Color::Black);
    resetHint.setOutlineThickness(1.5f);
    sf::FloatRect resetBounds = resetHint.getLocalBounds();
    resetHint.setOrigin(resetBounds.width / 2.f, resetBounds.height / 2.f);
    resetHint.setPosition(screenCenter.x, SCREEN_HEIGHT - 130.f);
    window.draw(resetHint);

    // Footer instructions
    sf::Text instructions;
    instructions.setFont(font);
    instructions.setString("Arrow Keys - Navigate | ENTER - Upgrade Selected Skill | Defeat enemies to earn skill points!");
    instructions.setCharacterSize(16);
    instructions.setFillColor(sf::Color(180, 180, 180));
    sf::FloatRect instrBounds = instructions.getLocalBounds();
    instructions.setOrigin(instrBounds.width / 2.f, instrBounds.height / 2.f);
    instructions.setPosition(screenCenter.x, SCREEN_HEIGHT - 80.f);
    window.draw(instructions);

    // Close hint
    sf::Text closeHint;
    closeHint.setFont(font);
    closeHint.setString("Press ESC or T to close");
    closeHint.setCharacterSize(20);
    closeHint.setFillColor(sf::Color(180, 180, 180));
    sf::FloatRect hintBounds = closeHint.getLocalBounds();
    closeHint.setOrigin(hintBounds.width / 2.f, hintBounds.height / 2.f);
    closeHint.setPosition(screenCenter.x, SCREEN_HEIGHT - 40.f);
    window.draw(closeHint);
}

// Help / controls page
void UISystem::renderHelpScreen(sf::RenderWindow &window)
{
    // UI view
    sf::View screenView(sf::FloatRect(0.f, 0.f, SCREEN_WIDTH, SCREEN_HEIGHT));
    window.setView(screenView);

    // Dim overlay
    sf::RectangleShape overlay(sf::Vector2f(SCREEN_WIDTH, SCREEN_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 210));
    window.draw(overlay);

    // Font + center
    const sf::Font &font = resourceManager.getFont();
    sf::Vector2f screenCenter(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f);

    // Title
    sf::Text titleText;
    titleText.setFont(font);
    titleText.setString("HELP");
    titleText.setCharacterSize(48);
    titleText.setFillColor(sf::Color(255, 200, 100));
    titleText.setOutlineColor(sf::Color::Black);
    titleText.setOutlineThickness(3.f);
    sf::FloatRect titleBounds = titleText.getLocalBounds();
    titleText.setOrigin(titleBounds.width / 2.f, titleBounds.height / 2.f);
    titleText.setPosition(screenCenter.x, 100.f);
    window.draw(titleText);

    // Control instructions body
    std::string controlsText =
        "MOVEMENT\n"
        "Arrow Keys - Move Left/Right\n"
        "SPACE - Jump (press twice for double jump)\n\n"
        "COMBAT\n"
        "A - Normal Attack\n"
        "S - Special Attack (costs stamina)\n"
        "F - Fireball Attack\n"
        "R - Switch Projectile Type (Fire/Ice)\n\n"
        "ITEMS\n"
        "Q - Use HP Potion (+30 HP)\n\n"
        "MENU\n"
        "ESC - Pause Menu\n"
        "D - Toggle Debug Mode";

    // Render instructions
    sf::Text controlsDisplay;
    controlsDisplay.setFont(font);
    controlsDisplay.setString(controlsText);
    controlsDisplay.setCharacterSize(22);
    controlsDisplay.setFillColor(sf::Color(220, 220, 220));
    controlsDisplay.setOutlineColor(sf::Color::Black);
    controlsDisplay.setOutlineThickness(1.f);
    controlsDisplay.setLineSpacing(1.3f);
    sf::FloatRect controlsBounds = controlsDisplay.getLocalBounds();
    controlsDisplay.setOrigin(controlsBounds.width / 2.f, 0.f);
    controlsDisplay.setPosition(screenCenter.x, 180.f);
    window.draw(controlsDisplay);

    // Exit hint
    sf::Text closeHint;
    closeHint.setFont(font);
    closeHint.setString("Press ESC to go back");
    closeHint.setCharacterSize(20);
    closeHint.setFillColor(sf::Color(180, 180, 180));
    sf::FloatRect hintBounds = closeHint.getLocalBounds();
    closeHint.setOrigin(hintBounds.width / 2.f, hintBounds.height / 2.f);
    closeHint.setPosition(screenCenter.x, SCREEN_HEIGHT - 50.f);
    window.draw(closeHint);
}

// Visualizes a singly linked list "combat log" with traversal, selection, and deletion hints
void UISystem::renderCombatLog(sf::RenderWindow &window,
                               SinglyLinkedList<std::string> &combatLog,
                               int combatLogCurrentNode,
                               int combatLogTraversalCount,
                               int combatLogDeleteCount)
{
    // UI view
    sf::View screenView(sf::FloatRect(0.f, 0.f, SCREEN_WIDTH, SCREEN_HEIGHT));
    window.setView(screenView);

    // Background plate (not fully dim to differentiate page)
    sf::RectangleShape overlay(sf::Vector2f(SCREEN_WIDTH, SCREEN_HEIGHT));
    overlay.setFillColor(sf::Color(20, 20, 30));
    window.draw(overlay);

    // Font + center
    const sf::Font &font = resourceManager.getFont();
    sf::Vector2f screenCenter(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f);

    // Title + layout metrics
    sf::Text titleText;
    titleText.setFont(font);
    titleText.setString("COMBAT LOG - Singly Linked List Demo");
    titleText.setCharacterSize(42);
    titleText.setFillColor(sf::Color::White); // Use white for title
    titleText.setOutlineColor(sf::Color::Black);
    titleText.setOutlineThickness(3.f);
    sf::FloatRect titleBounds = titleText.getLocalBounds();
    titleText.setOrigin(titleBounds.width / 2.f, titleBounds.height / 2.f);
    float titleY = 60.f;
    float closeHintY = SCREEN_HEIGHT - 50.f;
    float contentHeight = closeHintY - titleY;
    float contentStartY = titleY + contentHeight * 0.15f;
    titleText.setPosition(screenCenter.x, titleY);
    window.draw(titleText);

    // Info box strip with overall SLL stats (size, traversals, deletions)
    sf::RectangleShape infoBox(sf::Vector2f(700.f, 60.f));
    infoBox.setFillColor(sf::Color(40, 40, 60, 220));
    infoBox.setOutlineColor(sf::Color(100, 150, 255));
    infoBox.setOutlineThickness(2.f);
    infoBox.setPosition(screenCenter.x - 350.f, contentStartY);
    window.draw(infoBox);

    // Text display of SLL info
    sf::Text sllInfoText;
    sllInfoText.setFont(font);
    sllInfoText.setString("SLL: Forward-Only | Nodes: " + std::to_string(combatLog.size()) +
                          "/100 | Traversals: " + std::to_string(combatLogTraversalCount) +
                          " | Deletions: " + std::to_string(combatLogDeleteCount));
    sllInfoText.setCharacterSize(16);
    sllInfoText.setFillColor(sf::Color(150, 150, 150)); // Use gray for info
    sf::FloatRect infoBounds = sllInfoText.getLocalBounds();
    sllInfoText.setOrigin(infoBounds.width / 2.f, infoBounds.height / 2.f);
    sllInfoText.setPosition(screenCenter.x, contentStartY + 30.f);
    window.draw(sllInfoText);

    // If the log is empty, show a placeholder message
    if (combatLog.size() == 0)
    {
        sf::Text noLogsText;
        noLogsText.setFont(font);
        noLogsText.setString("No combat events yet...\nStart fighting to see logs!");
        noLogsText.setCharacterSize(24);
        noLogsText.setFillColor(sf::Color(150, 150, 150)); // Keep gray for no logs
        sf::FloatRect noLogsBounds = noLogsText.getLocalBounds();
        noLogsText.setOrigin(noLogsBounds.width / 2.f, noLogsBounds.height / 2.f);
        noLogsText.setPosition(screenCenter.x, screenCenter.y);
        window.draw(noLogsText);
    }
    else
    {
        // Node chain visualization controls
        float chainY = contentStartY + 70.f;
        float nodeWidth = 50.f;
        float spacing = 10.f;
        int maxVisibleNodes = 10;
        int startNode = std::max(0, combatLogCurrentNode - 4);
        int endNode = std::min(combatLog.size(), startNode + maxVisibleNodes);

        // Draw a windowed portion of nodes horizontally
        for (int i = startNode; i < endNode; i++)
        {
            float xPos = 100.f + (i - startNode) * (nodeWidth + spacing);

            // Node rectangle (selected node is white)
            sf::RectangleShape nodeBox(sf::Vector2f(nodeWidth, 40.f));
            nodeBox.setPosition(xPos, chainY);

            if (i == combatLogCurrentNode)
            {
                nodeBox.setFillColor(sf::Color::White); // Selected node: white
                nodeBox.setOutlineColor(sf::Color::Black);
                nodeBox.setOutlineThickness(3.f);
            }
            else
            {
                nodeBox.setFillColor(sf::Color(150, 150, 150, 200)); // Unselected: gray
                nodeBox.setOutlineColor(sf::Color::Black);
                nodeBox.setOutlineThickness(2.f);
            }
            window.draw(nodeBox);

            // Node index label
            sf::Text nodeNum;
            nodeNum.setFont(font);
            nodeNum.setString(std::to_string(i));
            nodeNum.setCharacterSize(16);
            nodeNum.setFillColor(sf::Color::Black); // Node number: black for contrast
            sf::FloatRect numBounds = nodeNum.getLocalBounds();
            nodeNum.setOrigin(numBounds.width / 2.f, numBounds.height / 2.f);
            nodeNum.setPosition(xPos + nodeWidth / 2.f, chainY + 20.f);
            window.draw(nodeNum);

            // Arrow line to the next node
            if (i < endNode - 1)
            {
                sf::RectangleShape arrow(sf::Vector2f(spacing, 3.f));
                arrow.setPosition(xPos + nodeWidth, chainY + 19.f);
                arrow.setFillColor(sf::Color(150, 150, 200));
                window.draw(arrow);

                sf::CircleShape arrowHead(4.f, 3);
                arrowHead.setPosition(xPos + nodeWidth + spacing - 4.f, chainY + 16.f);
                arrowHead.setFillColor(sf::Color(150, 150, 200));
                arrowHead.setRotation(90.f);
                window.draw(arrowHead);
            }
        }

        // Content display area for the selected node's message
        float contentY = chainY + 70.f;

        // Traverse to find the current node text (forward-only behavior)
        std::string currentMessage;
        int traverseIndex = 0;
        combatLog.forEach([&](const std::string &logEntry)
                          {
            if (traverseIndex == combatLogCurrentNode)
            {
                currentMessage = logEntry;
            }
            traverseIndex++; });

        // Box behind the selected node message
        sf::RectangleShape contentBox(sf::Vector2f(700.f, 100.f));
        contentBox.setFillColor(sf::Color(30, 30, 40, 240));
        contentBox.setOutlineColor(sf::Color::Black);
        contentBox.setOutlineThickness(3.f);
        contentBox.setPosition(screenCenter.x - 350.f, contentY);
        window.draw(contentBox);

        // Label for "Current Node [index]"
        sf::Text nodeLabel;
        nodeLabel.setFont(font);
        nodeLabel.setString("Current Node [" + std::to_string(combatLogCurrentNode) + "]:");
        nodeLabel.setCharacterSize(20);
        nodeLabel.setFillColor(sf::Color::White);
        nodeLabel.setPosition(screenCenter.x - 330.f, contentY + 15.f);
        window.draw(nodeLabel);

        // Selected node message body
        sf::Text messageText;
        messageText.setFont(font);
        messageText.setString(currentMessage);
        messageText.setCharacterSize(22);
        messageText.setFillColor(sf::Color::White);
        messageText.setPosition(screenCenter.x - 330.f, contentY + 50.f);
        window.draw(messageText);

        // Navigation legend for SLL behaviors and limitations
        float navY = contentY + 120.f;

        sf::Text navTitle;
        navTitle.setFont(font);
        navTitle.setString("SLL Navigation Demonstration:");
        navTitle.setCharacterSize(24);
        navTitle.setFillColor(sf::Color::White);
        navTitle.setPosition(100.f, navY);
        window.draw(navTitle);

        // Forward arrow availability (depends on not being at the end)
        sf::Text forwardArrow;
        forwardArrow.setFont(font);
        if (combatLogCurrentNode < combatLog.size() - 1)
        {
            forwardArrow.setString("RIGHT ARROW -> [Move Forward] - ENABLED");
            forwardArrow.setFillColor(sf::Color::White);
        }
        else
        {
            forwardArrow.setString("RIGHT ARROW -> [Move Forward] - At End");
            forwardArrow.setFillColor(sf::Color(150, 150, 150));
        }
        forwardArrow.setCharacterSize(18);
        forwardArrow.setPosition(120.f, navY + 40.f);
        window.draw(forwardArrow);

        // Backward movement not supported in SLL
        sf::Text backwardArrow;
        backwardArrow.setFont(font);
        backwardArrow.setString("LEFT ARROW  -> [Move Backward] - DISABLED (SLL limitation!)");
        backwardArrow.setCharacterSize(18);
        backwardArrow.setFillColor(sf::Color::White);
        backwardArrow.setPosition(120.f, navY + 70.f);
        window.draw(backwardArrow);

        // Reset to head (restart traversal)
        sf::Text upKey;
        upKey.setFont(font);
        upKey.setString("UP ARROW    -> [Reset to Head] - Restart traversal from beginning");
        upKey.setCharacterSize(18);
        upKey.setFillColor(sf::Color::White);
        upKey.setPosition(120.f, navY + 100.f);
        window.draw(upKey);

        // Delete the current node if any exist
        sf::Text deleteKey;
        deleteKey.setFont(font);
        if (combatLog.size() > 0)
        {
            deleteKey.setString("DELETE      -> [Remove Node] - Delete current node from list");
            deleteKey.setFillColor(sf::Color::White);
        }
        else
        {
            deleteKey.setString("DELETE      -> [Remove Node] - No nodes to delete");
            deleteKey.setFillColor(sf::Color(150, 150, 150));
        }
        deleteKey.setCharacterSize(18);
        deleteKey.setPosition(120.f, navY + 130.f);
        window.draw(deleteKey);
    }

    // Exit hint
    sf::Text closeHint;
    closeHint.setFont(font);
    closeHint.setString("Press ESC to go back");
    closeHint.setCharacterSize(20);
    closeHint.setFillColor(sf::Color(180, 180, 180));
    sf::FloatRect hintBounds = closeHint.getLocalBounds();
    closeHint.setOrigin(hintBounds.width / 2.f, hintBounds.height / 2.f);
    closeHint.setPosition(screenCenter.x, SCREEN_HEIGHT - 30.f);
    window.draw(closeHint);
}

// Top-level settings page
void UISystem::renderSettingsMenu(sf::RenderWindow &window,
                                  const GameStateData &settingsState)
{
    // UI view
    sf::View screenView(sf::FloatRect(0.f, 0.f, SCREEN_WIDTH, SCREEN_HEIGHT));
    window.setView(screenView);

    // Dim overlay
    sf::RectangleShape overlay(sf::Vector2f(SCREEN_WIDTH, SCREEN_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 210));
    window.draw(overlay);

    // Font + center
    const sf::Font &font = resourceManager.getFont();
    sf::Vector2f screenCenter(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f);

    // Title
    sf::Text title;
    title.setFont(font);
    title.setString("SETTINGS");
    title.setCharacterSize(48);
    title.setFillColor(sf::Color::White);
    title.setStyle(sf::Text::Bold);
    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setOrigin(titleBounds.width / 2.f, titleBounds.height / 2.f);
    title.setPosition(screenCenter.x, screenCenter.y - 150.f);
    window.draw(title);

    // Options list: audio, controls, help, back
    const char *options[] = {
        "Audio",
        "Controls",
        "Help",
        "Back"};
    const int numOptions = 4;

    for (int i = 0; i < numOptions; ++i)
    {
        sf::Text option;
        option.setFont(font);
        option.setString(options[i]);
        option.setCharacterSize(32);

        if (i == settingsState.selectedMenuOption)
        {
            // Selected
            option.setFillColor(sf::Color::White); // Selected: white
            option.setOutlineColor(sf::Color::Black);
            option.setOutlineThickness(2.f);
            option.setCharacterSize(36);
        }
        else
        {
            // Unselected
            option.setFillColor(sf::Color(150, 150, 150)); // Not selected: gray
            option.setOutlineColor(sf::Color::Black);
            option.setOutlineThickness(1.5f);
        }

        // Center row
        sf::FloatRect optionBounds = option.getLocalBounds();
        option.setOrigin(optionBounds.width / 2.f, optionBounds.height / 2.f);
        option.setPosition(screenCenter.x, screenCenter.y - 40.f + i * 60.f);
        window.draw(option);
    }

    // Bottom hint
    sf::Text controlsHint;
    controlsHint.setFont(font);
    controlsHint.setString("UP/DOWN: Navigate  |  ENTER: Select  |  ESC: Back");
    controlsHint.setCharacterSize(18);
    controlsHint.setFillColor(sf::Color(180, 180, 180));
    sf::FloatRect hintBounds = controlsHint.getLocalBounds();
    controlsHint.setOrigin(hintBounds.width / 2.f, hintBounds.height / 2.f);
    controlsHint.setPosition(screenCenter.x, screenCenter.y + 300.f);
    window.draw(controlsHint);
}

// Audio settings sliders (music + SFX) and reset/back options
void UISystem::renderAudioSettings(sf::RenderWindow &window,
                                   const GameStateData &audioState)
{
    // Background overlay only (this screen likely follows a parent that set the view)
    sf::RectangleShape overlay(sf::Vector2f(SCREEN_WIDTH, SCREEN_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 210));
    window.draw(overlay);

    // Title
    sf::Text title;
    title.setFont(resourceManager.getFont());
    title.setString("AUDIO");
    title.setCharacterSize(48);
    title.setFillColor(sf::Color::White);
    title.setStyle(sf::Text::Bold);
    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setOrigin(titleBounds.width / 2.f, titleBounds.height / 2.f);
    title.setPosition(SCREEN_WIDTH / 2.f, 150.f);
    window.draw(title);

    // Music volume label
    sf::Text musicLabel;
    musicLabel.setFont(resourceManager.getFont());
    musicLabel.setString("Music Volume:");
    musicLabel.setCharacterSize(28);
    musicLabel.setFillColor(audioState.selectedMenuOption == 0 ? sf::Color::White : sf::Color(150, 150, 150));
    if (audioState.selectedMenuOption == 0)
        musicLabel.setStyle(sf::Text::Bold);
    sf::FloatRect musicLabelBounds = musicLabel.getLocalBounds();
    musicLabel.setOrigin(musicLabelBounds.width / 2.f, musicLabelBounds.height / 2.f);
    musicLabel.setPosition(SCREEN_WIDTH / 2.f - 100.f, 250.f);
    window.draw(musicLabel);

    // Music volume bar background + fill
    sf::RectangleShape musicBarBg(sf::Vector2f(200.f, 20.f));
    musicBarBg.setFillColor(sf::Color(50, 50, 50));
    musicBarBg.setPosition(SCREEN_WIDTH / 2.f - 100.f, 280.f);
    window.draw(musicBarBg);

    sf::RectangleShape musicBar(sf::Vector2f(audioState.musicVolume * 2.f, 20.f));
    musicBar.setFillColor(sf::Color(100, 200, 100));
    musicBar.setPosition(SCREEN_WIDTH / 2.f - 100.f, 280.f);
    window.draw(musicBar);

    // Music value %
    sf::Text musicValue;
    musicValue.setFont(resourceManager.getFont());
    musicValue.setString(std::to_string(static_cast<int>(audioState.musicVolume)) + "%");
    musicValue.setCharacterSize(24);
    musicValue.setFillColor(sf::Color::White);
    sf::FloatRect musicValueBounds = musicValue.getLocalBounds();
    musicValue.setOrigin(musicValueBounds.width / 2.f, musicValueBounds.height / 2.f);
    musicValue.setPosition(SCREEN_WIDTH / 2.f + 120.f, 265.f);
    window.draw(musicValue);

    // SFX volume label
    sf::Text sfxLabel;
    sfxLabel.setFont(resourceManager.getFont());
    sfxLabel.setString("SFX Volume:");
    sfxLabel.setCharacterSize(28);
    sfxLabel.setFillColor(audioState.selectedMenuOption == 1 ? sf::Color::White : sf::Color(150, 150, 150));
    if (audioState.selectedMenuOption == 1)
        sfxLabel.setStyle(sf::Text::Bold);
    sf::FloatRect sfxLabelBounds = sfxLabel.getLocalBounds();
    sfxLabel.setOrigin(sfxLabelBounds.width / 2.f, sfxLabelBounds.height / 2.f);
    sfxLabel.setPosition(SCREEN_WIDTH / 2.f - 100.f, 350.f);
    window.draw(sfxLabel);

    // SFX volume bar background + fill
    sf::RectangleShape sfxBarBg(sf::Vector2f(200.f, 20.f));
    sfxBarBg.setFillColor(sf::Color(50, 50, 50));
    sfxBarBg.setPosition(SCREEN_WIDTH / 2.f - 100.f, 380.f);
    window.draw(sfxBarBg);

    sf::RectangleShape sfxBar(sf::Vector2f(audioState.sfxVolume * 2.f, 20.f));
    sfxBar.setFillColor(sf::Color(200, 100, 100));
    sfxBar.setPosition(SCREEN_WIDTH / 2.f - 100.f, 380.f);
    window.draw(sfxBar);

    // SFX value %
    sf::Text sfxValue;
    sfxValue.setFont(resourceManager.getFont());
    sfxValue.setString(std::to_string(static_cast<int>(audioState.sfxVolume)) + "%");
    sfxValue.setCharacterSize(24);
    sfxValue.setFillColor(sf::Color::White);
    sf::FloatRect sfxValueBounds = sfxValue.getLocalBounds();
    sfxValue.setOrigin(sfxValueBounds.width / 2.f, sfxValueBounds.height / 2.f);
    sfxValue.setPosition(SCREEN_WIDTH / 2.f + 120.f, 365.f);
    window.draw(sfxValue);

    // Reset-to-defaults option
    sf::Text resetOption;
    resetOption.setFont(resourceManager.getFont());
    resetOption.setString("Reset to Default");
    resetOption.setCharacterSize(28);
    resetOption.setFillColor(audioState.selectedMenuOption == 2 ? sf::Color::White : sf::Color(150, 150, 150));
    if (audioState.selectedMenuOption == 2)
        resetOption.setStyle(sf::Text::Bold);
    sf::FloatRect resetBounds = resetOption.getLocalBounds();
    resetOption.setOrigin(resetBounds.width / 2.f, resetBounds.height / 2.f);
    resetOption.setPosition(SCREEN_WIDTH / 2.f, 450.f);
    window.draw(resetOption);

    // Back option
    sf::Text backOption;
    backOption.setFont(resourceManager.getFont());
    backOption.setString("Back");
    backOption.setCharacterSize(28);
    backOption.setFillColor(audioState.selectedMenuOption == 3 ? sf::Color::White : sf::Color(150, 150, 150));
    if (audioState.selectedMenuOption == 3)
        backOption.setStyle(sf::Text::Bold);
    sf::FloatRect backBounds = backOption.getLocalBounds();
    backOption.setOrigin(backBounds.width / 2.f, backBounds.height / 2.f);
    backOption.setPosition(SCREEN_WIDTH / 2.f, 500.f);
    window.draw(backOption);

    // Controls hint
    sf::Text controlsHint;
    controlsHint.setFont(resourceManager.getFont());
    controlsHint.setString("Use UP/DOWN to navigate, LEFT/RIGHT to adjust volume, ENTER to select, ESC to go back");
    controlsHint.setCharacterSize(18);
    controlsHint.setFillColor(sf::Color(180, 180, 180));
    sf::FloatRect hintBounds = controlsHint.getLocalBounds();
    controlsHint.setOrigin(hintBounds.width / 2.f, hintBounds.height / 2.f);
    controlsHint.setPosition(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT - 50.f);
    window.draw(controlsHint);
}

// Generic yes/no confirm dialog for quitting, restarting, or overwriting saves
void UISystem::renderConfirmDialog(sf::RenderWindow &window,
                                   const GameStateData &confirmState)
{
    // Background dim
    sf::RectangleShape overlay(sf::Vector2f(SCREEN_WIDTH, SCREEN_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 210));
    window.draw(overlay);

    // Modal box
    sf::RectangleShape dialogBox(sf::Vector2f(500.f, 200.f));
    dialogBox.setFillColor(sf::Color(40, 40, 50));
    dialogBox.setOutlineColor(sf::Color::White);
    dialogBox.setOutlineThickness(2.f);
    dialogBox.setPosition((SCREEN_WIDTH - 500.f) / 2.f, (SCREEN_HEIGHT - 200.f) / 2.f);
    window.draw(dialogBox);

    // Title that depends on confirm type
    sf::Text title;
    title.setFont(resourceManager.getFont());

    if (confirmState.type == GameStateType::ConfirmQuitToMenu)
        title.setString("Quit to Main Menu?");
    else if (confirmState.type == GameStateType::ConfirmRestart)
        title.setString("Restart Game?");
    else if (confirmState.type == GameStateType::ConfirmOverwriteSave)
        title.setString("Overwrite existing save?");

    title.setCharacterSize(32);
    title.setFillColor(sf::Color::White);
    title.setStyle(sf::Text::Bold);
    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setOrigin(titleBounds.width / 2.f, titleBounds.height / 2.f);
    title.setPosition(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 40.f);
    window.draw(title);

    // "Yes" option
    sf::Text yesOption;
    yesOption.setFont(resourceManager.getFont());
    yesOption.setString("Yes");
    yesOption.setCharacterSize(28);
    yesOption.setFillColor(confirmState.selectedMenuOption == 0 ? sf::Color::White : sf::Color(150, 150, 150));
    if (confirmState.selectedMenuOption == 0)
        yesOption.setStyle(sf::Text::Bold);
    sf::FloatRect yesBounds = yesOption.getLocalBounds();
    yesOption.setOrigin(yesBounds.width / 2.f, yesBounds.height / 2.f);
    yesOption.setPosition(SCREEN_WIDTH / 2.f - 80.f, SCREEN_HEIGHT / 2.f + 20.f);
    window.draw(yesOption);

    // "No" option
    sf::Text noOption;
    noOption.setFont(resourceManager.getFont());
    noOption.setString("No");
    noOption.setCharacterSize(28);
    noOption.setFillColor(confirmState.selectedMenuOption == 1 ? sf::Color::White : sf::Color(150, 150, 150));
    if (confirmState.selectedMenuOption == 1)
        noOption.setStyle(sf::Text::Bold);
    sf::FloatRect noBounds = noOption.getLocalBounds();
    noOption.setOrigin(noBounds.width / 2.f, noBounds.height / 2.f);
    noOption.setPosition(SCREEN_WIDTH / 2.f + 80.f, SCREEN_HEIGHT / 2.f + 20.f);
    window.draw(noOption);

    // Controls hint
    sf::Text controlsHint;
    controlsHint.setFont(resourceManager.getFont());
    controlsHint.setString("Use LEFT/RIGHT to select, ENTER to confirm, ESC for No");
    controlsHint.setCharacterSize(18);
    controlsHint.setFillColor(sf::Color(180, 180, 180));
    sf::FloatRect hintBounds = controlsHint.getLocalBounds();
    controlsHint.setOrigin(hintBounds.width / 2.f, hintBounds.height / 2.f);
    controlsHint.setPosition(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 80.f);
    window.draw(controlsHint);
}

// Save game menu: choose a slot to save into (shows empty/non-empty)
void UISystem::renderSaveGameMenu(sf::RenderWindow &window,
                                  const GameStateData &saveState,
                                  SaveGameManager *saveManager)
{
    // Dim background
    sf::RectangleShape overlay(sf::Vector2f(SCREEN_WIDTH, SCREEN_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 210));
    window.draw(overlay);

    // Font
    const sf::Font &font = resourceManager.getFont();

    // Title
    sf::Text title;
    title.setFont(font);
    title.setString("SAVE GAME");
    title.setCharacterSize(48);
    title.setFillColor(sf::Color::White);
    title.setStyle(sf::Text::Bold);
    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setOrigin(titleBounds.width / 2.f, titleBounds.height / 2.f);
    title.setPosition(SCREEN_WIDTH / 2.f, 150.f);
    window.draw(title);

    // Options: 3 save slots + back
    const char *saveSlots[] = {"Save Slot 1", "Save Slot 2", "Save Slot 3", "Back"};
    const int numOptions = 4;

    // Query if each slot has an existing save
    bool slotAvailable[4] = {false, false, false, true};
    if (saveManager)
    {
        slotAvailable[0] = saveManager->saveExists("savegame1");
        slotAvailable[1] = saveManager->saveExists("savegame2");
        slotAvailable[2] = saveManager->saveExists("savegame3");
    }

    // Draw options with "(Empty)" for missing saves
    for (int i = 0; i < numOptions; ++i)
    {
        sf::Text option;
        option.setFont(font);
        std::string displayText = saveSlots[i];
        if (i < 3 && !slotAvailable[i])
        {
            displayText += " (Empty)";
        }
        option.setString(displayText);
        option.setCharacterSize(32);

        if (i == saveState.selectedMenuOption)
        {
            option.setFillColor(sf::Color::White); // Selected: white
            option.setOutlineColor(sf::Color::Black);
            option.setOutlineThickness(2.f);
            option.setCharacterSize(36);
        }
        else
        {
            option.setFillColor(sf::Color(150, 150, 150)); // Not selected: gray
            option.setOutlineColor(sf::Color::Black);
            option.setOutlineThickness(1.5f);
        }

        // Center row
        sf::FloatRect optionBounds = option.getLocalBounds();
        option.setOrigin(optionBounds.width / 2.f, optionBounds.height / 2.f);
        option.setPosition(SCREEN_WIDTH / 2.f, 280.f + i * 70.f);
        window.draw(option);
    }

    // Bottom hint
    sf::Text hint;
    hint.setFont(font);
    hint.setString("UP/DOWN: Navigate | ENTER: Save | ESC: Back");
    hint.setCharacterSize(18);
    hint.setFillColor(sf::Color(180, 180, 180));
    sf::FloatRect hintBounds = hint.getLocalBounds();
    hint.setOrigin(hintBounds.width / 2.f, hintBounds.height / 2.f);
    hint.setPosition(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT - 50.f);
    window.draw(hint);
}

// Load game menu: choose a slot to load from (disabled if empty)
void UISystem::renderLoadGameMenu(sf::RenderWindow &window,
                                  const GameStateData &loadState,
                                  SaveGameManager *saveManager)
{
    // Dim overlay
    sf::RectangleShape overlay(sf::Vector2f(SCREEN_WIDTH, SCREEN_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 210));
    window.draw(overlay);

    // Font
    const sf::Font &font = resourceManager.getFont();

    // Title
    sf::Text title;
    title.setFont(font);
    title.setString("LOAD GAME");
    title.setCharacterSize(48);
    title.setFillColor(sf::Color::White);
    title.setStyle(sf::Text::Bold);
    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setOrigin(titleBounds.width / 2.f, titleBounds.height / 2.f);
    title.setPosition(SCREEN_WIDTH / 2.f, 150.f);
    window.draw(title);

    // Options: 3 load slots + back
    const char *saveSlots[] = {"Load Slot 1", "Load Slot 2", "Load Slot 3", "Back"};
    const int numOptions = 4;

    // Query existing saves
    bool slotAvailable[4] = {false, false, false, true};
    if (saveManager)
    {
        slotAvailable[0] = saveManager->saveExists("savegame1");
        slotAvailable[1] = saveManager->saveExists("savegame2");
        slotAvailable[2] = saveManager->saveExists("savegame3");
    }

    // Draw options, disabling empty slots
    for (int i = 0; i < numOptions; ++i)
    {
        sf::Text option;
        option.setFont(font);

        std::string displayText = saveSlots[i];
        if (i < 3 && !slotAvailable[i])
        {
            displayText += " (Empty)";
        }

        option.setString(displayText);
        option.setCharacterSize(32);

        if (i == loadState.selectedMenuOption && slotAvailable[i])
        {
            option.setFillColor(sf::Color::White); // Selected: white
            option.setOutlineColor(sf::Color::Black);
            option.setOutlineThickness(2.f);
            option.setCharacterSize(36);
        }
        else if (!slotAvailable[i] && i < 3)
        {
            // Disabled look for empty slots
            option.setFillColor(sf::Color(80, 80, 80));
            option.setOutlineColor(sf::Color(50, 50, 50));
            option.setOutlineThickness(1.5f);
        }
        else
        {
            option.setFillColor(sf::Color(150, 150, 150)); // Not selected: gray
            option.setOutlineColor(sf::Color::Black);
            option.setOutlineThickness(1.5f);
        }

        // Center row
        sf::FloatRect optionBounds = option.getLocalBounds();
        option.setOrigin(optionBounds.width / 2.f, optionBounds.height / 2.f);
        option.setPosition(SCREEN_WIDTH / 2.f, 280.f + i * 70.f);
        window.draw(option);
    }

    // Bottom hint
    sf::Text hint;
    hint.setFont(font);
    hint.setString("UP/DOWN: Navigate | ENTER: Load | ESC: Back");
    hint.setCharacterSize(18);
    hint.setFillColor(sf::Color(180, 180, 180));
    sf::FloatRect hintBounds = hint.getLocalBounds();
    hint.setOrigin(hintBounds.width / 2.f, hintBounds.height / 2.f);
    hint.setPosition(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT - 50.f);
    window.draw(hint);
}

// Toast helpers: message API for transient centered popups
void UISystem::showToast(const std::string &message, sf::Color color)
{
    // Set new toast text and start the timer
    toastMessage = message;
    toastTimer = TOAST_DURATION;
    toastColor = color;
}

// Reduce toast time by dt until it reaches zero
void UISystem::updateToast(float dt)
{
    if (toastTimer > 0.0f)
    {
        toastTimer -= dt;
        if (toastTimer < 0.0f)
            toastTimer = 0.0f;
    }
}

// Render the toast if active (fades out during last 0.5s)
void UISystem::renderToast(sf::RenderWindow &window)
{
    // No active toast
    if (toastTimer <= 0.0f || toastMessage.empty())
        return;

    // UI view
    sf::View screenView(sf::FloatRect(0.f, 0.f, SCREEN_WIDTH, SCREEN_HEIGHT));
    window.setView(screenView);

    // Font
    const sf::Font &font = resourceManager.getFont();

    // Fade alpha near the end of lifetime
    float alpha = 255.0f;
    if (toastTimer < 0.5f)
        alpha = (toastTimer / 0.5f) * 255.0f;

    // Message text
    sf::Text toastText;
    toastText.setFont(font);
    toastText.setString(toastMessage);
    toastText.setCharacterSize(24);
    toastText.setFillColor(sf::Color(toastColor.r, toastColor.g, toastColor.b, static_cast<sf::Uint8>(alpha)));
    toastText.setOutlineColor(sf::Color(0, 0, 0, static_cast<sf::Uint8>(alpha)));
    toastText.setOutlineThickness(2.f);

    // Center text metrics (compensate for top offset within bounds)
    sf::FloatRect textBounds = toastText.getLocalBounds();
    toastText.setOrigin(textBounds.width / 2.f, textBounds.height / 2.f + textBounds.top);
    toastText.setPosition(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f);

    // Background behind toast
    sf::RectangleShape toastBg(sf::Vector2f(textBounds.width + 40.f, textBounds.height + 30.f));
    toastBg.setFillColor(sf::Color(0, 0, 0, static_cast<sf::Uint8>(alpha * 0.7f)));
    toastBg.setOutlineColor(sf::Color(toastColor.r, toastColor.g, toastColor.b, static_cast<sf::Uint8>(alpha)));
    toastBg.setOutlineThickness(2.f);
    toastBg.setOrigin(toastBg.getSize().x / 2.f, toastBg.getSize().y / 2.f);
    toastBg.setPosition(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f);

    // Draw background then text
    window.draw(toastBg);
    window.draw(toastText);
}

// Controls remapping menu (bind keys to GameAction entries)
void UISystem::renderControlsMenu(sf::RenderWindow &window,
                                  const GameStateData &controlsState,
                                  const KeyBindingManager &keyManager)
{
    // Dim overlay
    sf::RectangleShape overlay(sf::Vector2f(SCREEN_WIDTH, SCREEN_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 210));
    window.draw(overlay);

    // Header: either "PRESS A KEY..." or "CONTROLS"
    sf::Text title;
    title.setFont(resourceManager.getFont());
    title.setString(controlsState.awaitingKeyPress ? "PRESS A KEY..." : "CONTROLS");
    title.setCharacterSize(48);
    title.setFillColor(controlsState.awaitingKeyPress ? sf::Color::Yellow : sf::Color::White);
    title.setStyle(sf::Text::Bold);
    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setOrigin(titleBounds.width / 2.f, titleBounds.height / 2.f);
    title.setPosition(SCREEN_WIDTH / 2.f, 80.f);
    window.draw(title);

    // If waiting for key input, show a simple instruction and stop
    if (controlsState.awaitingKeyPress)
    {
        sf::Text instruction;
        instruction.setFont(resourceManager.getFont());
        instruction.setString("Press any key to bind, or ESC to cancel");
        instruction.setCharacterSize(24);
        instruction.setFillColor(sf::Color::White);
        sf::FloatRect instructionBounds = instruction.getLocalBounds();
        instruction.setOrigin(instructionBounds.width / 2.f, instructionBounds.height / 2.f);
        instruction.setPosition(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f);
        window.draw(instruction);
        return;
    }

    // Structure to pair actions with UI category headers and labels
    struct ActionDisplay
    {
        GameAction action;
        const char *categoryName;
        const char *displayName;
    };

    // Actions list with category separators
    const ActionDisplay actions[] = {
        // Movement
        {GameAction::MOVE_LEFT, "Movement", "Move Left"},
        {GameAction::MOVE_RIGHT, "", "Move Right"},
        {GameAction::JUMP, "", "Jump"},

        // Combat
        {GameAction::SHOOT_PROJECTILE, "Combat", "Shoot Projectile"},
        {GameAction::SWITCH_PROJECTILE, "", "Switch Projectile"},
        {GameAction::MELEE_ATTACK, "", "Melee Attack"},
        {GameAction::SPECIAL_ATTACK, "", "Special Attack"},

        // Items
        {GameAction::USE_POTION, "Items", "Use HP Potion"},

        // Game
        {GameAction::OPEN_SKILL_TREE, "Game", "Open Skill Tree"},
    };
    const int numActions = sizeof(actions) / sizeof(actions[0]);

    // Layout constants for the 2-column mapping view
    const float startY = 140.f;
    const float rowHeight = 35.f;
    const float labelX = SCREEN_WIDTH / 2.f - 200.f; // Action label column
    const float keyX = SCREEN_WIDTH / 2.f + 100.f;   // Bound key column

    int currentRow = 0;
    std::string lastCategory = "";

    // Draw each action, with category header rows when needed
    for (int i = 0; i < numActions; ++i)
    {
        // Category header (non-empty string and different from previous)
        if (actions[i].categoryName[0] != '\0' && lastCategory != actions[i].categoryName)
        {
            sf::Text category;
            category.setFont(resourceManager.getFont());
            category.setString(actions[i].categoryName);
            category.setCharacterSize(28);
            category.setFillColor(sf::Color(100, 200, 255));
            category.setStyle(sf::Text::Bold | sf::Text::Underlined);
            category.setPosition(labelX - 50.f, startY + currentRow * rowHeight);
            window.draw(category);

            lastCategory = actions[i].categoryName;
            currentRow++;
        }

        // Determine if this row is currently selected
        bool isSelected = (i == controlsState.selectedMenuOption);

        // Left column: action name
        sf::Text actionText;
        actionText.setFont(resourceManager.getFont());
        actionText.setString(actions[i].displayName);
        actionText.setCharacterSize(22);
        actionText.setFillColor(isSelected ? sf::Color::White : sf::Color(150, 150, 150));
        if (isSelected)
            actionText.setStyle(sf::Text::Bold);
        actionText.setPosition(labelX, startY + currentRow * rowHeight);
        window.draw(actionText);

        // Right column: key currently bound to this action
        sf::Keyboard::Key currentKey = keyManager.getKeyForAction(actions[i].action);
        std::string keyName = KeyBindingManager::getKeyName(currentKey);

        sf::Text keyText;
        keyText.setFont(resourceManager.getFont());
        keyText.setString("[ " + keyName + " ]");
        keyText.setCharacterSize(22);
        keyText.setFillColor(isSelected ? sf::Color::White : sf::Color(180, 180, 180));
        if (isSelected)
            keyText.setStyle(sf::Text::Bold);
        keyText.setPosition(keyX, startY + currentRow * rowHeight);
        window.draw(keyText);

        currentRow++;
    }

    // Bottom action buttons: Reset and Back
    const float bottomY = SCREEN_HEIGHT - 120.f;
    const int resetOption = numActions;
    const int backOption = numActions + 1;

    // Reset to defaults
    sf::Text resetText;
    resetText.setFont(resourceManager.getFont());
    resetText.setString("Reset to Defaults");
    resetText.setCharacterSize(28);
    resetText.setFillColor(controlsState.selectedMenuOption == resetOption ? sf::Color::White : sf::Color(150, 150, 150));
    if (controlsState.selectedMenuOption == resetOption)
        resetText.setStyle(sf::Text::Bold);
    sf::FloatRect resetBounds = resetText.getLocalBounds();
    resetText.setOrigin(resetBounds.width / 2.f, resetBounds.height / 2.f);
    resetText.setPosition(SCREEN_WIDTH / 2.f, bottomY);
    window.draw(resetText);

    // Back
    sf::Text backText;
    backText.setFont(resourceManager.getFont());
    backText.setString("Back");
    backText.setCharacterSize(28);
    backText.setFillColor(controlsState.selectedMenuOption == backOption ? sf::Color::White : sf::Color(150, 150, 150));
    if (controlsState.selectedMenuOption == backOption)
        backText.setStyle(sf::Text::Bold);
    sf::FloatRect backBounds = backText.getLocalBounds();
    backText.setOrigin(backBounds.width / 2.f, backBounds.height / 2.f);
    backText.setPosition(SCREEN_WIDTH / 2.f, bottomY + 40.f);
    window.draw(backText);

    // Footer hint
    sf::Text controlsHint;
    controlsHint.setFont(resourceManager.getFont());
    controlsHint.setString("UP/DOWN: Navigate | ENTER: Rebind | ESC: Back");
    controlsHint.setCharacterSize(18);
    controlsHint.setFillColor(sf::Color(180, 180, 180));
    sf::FloatRect hintBounds = controlsHint.getLocalBounds();
    controlsHint.setOrigin(hintBounds.width / 2.f, hintBounds.height / 2.f);
    controlsHint.setPosition(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT - 30.f);
    window.draw(controlsHint);
}
