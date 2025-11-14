#include "Session.h"
#include "DebugRenderer.h"
#include <algorithm>
#include <cmath>
#include <iostream>

// ==============================
// Constructors / Destructors
// ==============================

Session::Session()
    : window(sf::VideoMode(static_cast<unsigned int>(SCREEN_WIDTH), static_cast<unsigned int>(SCREEN_HEIGHT)),
             "SFML Action Prototype"),
      gameWorld(ResourceManager::getInstance()),
      cameraController(SCREEN_WIDTH, SCREEN_HEIGHT, WORLD_WIDTH),
      uiSystem(ResourceManager::getInstance()),
      combatSystem(ResourceManager::getInstance(), inputManager, [this](const std::string &msg)
                   { addCombatLog(msg); }, [this](int damage, sf::Vector2f position, const sf::Color &color, bool isHealing)
                   { spawnFloatingText(damage, position, color, isHealing); }, [this](GameStateType type)
                   { pushState(type); })
{
    window.setFramerateLimit(120); // Limit FPS to stabilize timing
}

Session::~Session()
{
    // Ensure we clean up any queued floating texts
    clearFloatingTexts("[Session] Cleaned up floating texts in destructor");
}

// ==============================
// Initialization / Reset
// ==============================

bool Session::initialize()
{
    // Load all resources (textures, fonts, audio, etc.)
    if (!ResourceManager::getInstance().loadAll())
    {
        std::cerr << "Failed to load game resources" << std::endl;
        return false;
    }

    // Start background music
    ResourceManager::getInstance().getBackgroundMusic().play();

    // Initialize world content (tiles, entities, etc.)
    if (!gameWorld.initialize())
    {
        std::cerr << "Failed to initialize game world" << std::endl;
        return false;
    }

    // Initialize higher-level game logic controller
    if (!gameMaster.initialize())
    {
        std::cerr << "Warning: Failed to initialize GameMaster" << std::endl;
    }

    return true;
}

void Session::resetGame()
{
    // Reset world to initial state
    gameWorld.reset();

    // Reset per-enemy damage application tracking
    enemyAttackDamageApplied = std::make_unique<bool[]>(gameWorld.getEnemyCount());
    for (std::size_t i = 0; i < gameWorld.getEnemyCount(); ++i)
    {
        enemyAttackDamageApplied[i] = false;
    }
    bossAttackDamageApplied = false;

    // Reset win/lose state flags
    gameOver = false;
    gameWon = false;

    // Clear transient UI elements and logs
    clearFloatingTexts("[Session] Cleared all floating texts during reset");
    clearCombatLog();
    addCombatLog("Game Started!");

    // Reset long-running systems
    gameMaster.reset();

    // Reset the combat system with a valid player reference
    Player *player = gameWorld.getPlayer();
    if (player)
        combatSystem.reset(rng, *player);
    else
        combatSystem.reset(rng, *gameWorld.getPlayer());

    // Clear latched inputs
    inputManager.setProjectileKeyPressed(false);
    inputManager.setSwitchProjectileKeyPressed(false);
    inputManager.setAttack1KeyPressed(false);
    inputManager.setAttack2KeyPressed(false);

    // Allow a fresh game-over sound on next end state
    gameOverSoundPlayed = false;
}

// ==============================
// Main Loop
// ==============================

void Session::run()
{
    // Abort if initialization fails
    if (!initialize())
    {
        return;
    }

    // Enter the main menu as the initial state
    stateStack.push(GameStateData(GameStateType::MainMenu));

    // Prepare game state before first frame
    resetGame();

    // Primary application/game loop
    while (window.isOpen() && !requestExit)
    {
        processEvents(); // Handle inputs and window events

        GameStateType currentState = getCurrentState();

        // Render Main Menu if active
        if (currentState == GameStateType::MainMenu)
        {
            if (!stateStack.isEmpty())
            {
                uiSystem.renderMainMenu(window, stateStack.top());
            }
            window.display();
            continue;
        }

        // Save Game Menu overlay
        if (currentState == GameStateType::SaveGameMenu)
        {
            window.clear();
            renderWorld();
            uiSystem.renderSaveGameMenu(window, stateStack.top(), &saveGameManager);
            window.display();
            continue;
        }

        // Load Game Menu overlay
        if (currentState == GameStateType::LoadGameMenu)
        {
            window.clear();
            uiSystem.renderLoadGameMenu(window, stateStack.top(), &saveGameManager);
            window.display();
            continue;
        }

        // Clamp delta time to avoid large physics steps
        const float dt = std::min(deltaClock.restart().asSeconds(), 0.1f);

        // Evaluate end conditions
        checkGameOver();

        // If game has ended, render end screen
        if (gameOver)
        {
            renderGameOver();
            window.display();
            continue;
        }

        // Normal frame update + render
        update(dt);
        render();
        window.display();
    }
}

// ==============================
// Event Processing
// ==============================

void Session::processEvents()
{
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            // Window close button
            window.close();
        }
        else if (event.type == sf::Event::Resized)
        {
            // Keep camera aspect and center after resize
            const sf::Vector2f newSize{static_cast<float>(event.size.width), static_cast<float>(event.size.height)};
            cameraController.handleResize(newSize.x, newSize.y);
            cameraController.applyToWindow(window);
        }
        else if (event.type == sf::Event::KeyPressed)
        {
            // Toggle debug overlay in Playing state
            if (event.key.code == sf::Keyboard::D && getCurrentState() == GameStateType::Playing)
            {
                debugMode = !debugMode;
            }

            // For debug: remember last key
            if (debugMode)
            {
                lastPressedKey = event.key.code;
                lastPressedKeyName = KeyBindingManager::getKeyName(event.key.code);
            }

            // Route to input manager with current context
            inputManager.processEvent(event, stateStack, gameOver, gameWorld.getPlayer(), ResourceManager::getInstance(),
                                      combatLog,
                                      combatLogCurrentNode, combatLogTraversalCount, combatLogDeleteCount, requestExit, &uiSystem, &keyBindingManager, &saveGameManager);

            // Handle state transitions that require immediate actions
            if (!stateStack.isEmpty())
            {
                GameStateType currentState = stateStack.top().type;

                static GameStateType previousState = GameStateType::MainMenu;

                // If confirmed restart -> reset gameplay
                if (currentState == GameStateType::Playing && previousState == GameStateType::ConfirmRestart)
                {
                    resetGame();
                }

                // Return to main menu clears game-over flags and resets
                if (currentState == GameStateType::MainMenu && gameOver)
                {
                    resetGame();
                    gameOver = false;
                    gameWon = false;
                }

                // Confirm quit to menu also resets flags/state
                if (currentState == GameStateType::MainMenu &&
                    (previousState == GameStateType::ConfirmQuitToMenu))
                {
                    resetGame();
                    gameOver = false;
                    gameWon = false;
                }

                previousState = currentState;
            }
        }
        else
        {
            // Forward non-keypress events (mouse, key releases, etc.)
            inputManager.processEvent(event, stateStack, gameOver, gameWorld.getPlayer(), ResourceManager::getInstance(),
                                      combatLog,
                                      combatLogCurrentNode, combatLogTraversalCount, combatLogDeleteCount, requestExit, &uiSystem, &keyBindingManager, &saveGameManager);
        }
    }
}

// ==============================
// Per-Frame Update (Game Logic)
// ==============================

void Session::update(float dt)
{
    // Update transient UI toasts
    uiSystem.updateToast(dt);

    // Boss death slow-mo branch (timescale reduced)
    if (getCurrentState() == GameStateType::BossDeathSlowMotion)
    {
        bossDeathSlowMotionTimer -= dt;

        float slowDt = dt * SLOWMO_TIME_SCALE;

        updatePlayer(slowDt);
        combatSystem.updateProjectiles(slowDt,
                                       *gameWorld.getPlayer(),
                                       gameWorld.getEnemies(),
                                       gameWorld.getEnemyCount(),
                                       gameWorld.getBoss(),
                                       gameWorld.isBossSpawned(),
                                       bossDeathSlowMotionActive,
                                       bossDeathSlowMotionTimer,
                                       BOSS_DEATH_SLOWMO_DURATION,
                                       &keyBindingManager);
        updateBoss(slowDt);
        updateFloatingTexts(slowDt);
        cameraController.followPlayer(gameWorld.getPlayer());

        // End slow-mo and mark victory when timer elapses
        if (bossDeathSlowMotionTimer <= 0.f)
        {
            bossDeathSlowMotionActive = false;
            popState();
            gameWon = true;

            ResourceManager::getInstance().stopWalkSound();
            ResourceManager::getInstance().playVictorySound();
        }
        return;
    }

    // If paused (via state machine), skip simulation
    if (isGamePaused())
    {
        return;
    }

    // Accumulate timer for low HP warning visuals
    lowHpWarningTimer += dt;

    // High-level game rules (rage, events, etc.)
    gameMaster.update(dt, *gameWorld.getPlayer(), gameWorld);

    // Player update and combat handling
    updatePlayer(dt);
    combatSystem.handlePlayerAttacks(*gameWorld.getPlayer(),
                                     gameWorld.getEnemies(),
                                     gameWorld.getEnemyCount(),
                                     gameWorld.getBoss(),
                                     gameWorld.isBossSpawned(),
                                     bossDeathSlowMotionActive,
                                     bossDeathSlowMotionTimer,
                                     BOSS_DEATH_SLOWMO_DURATION,
                                     &keyBindingManager,
                                     gameMaster.getPlayerDamageMultiplier());

    // Projectiles, items, hazards
    combatSystem.updateProjectiles(dt,
                                   *gameWorld.getPlayer(),
                                   gameWorld.getEnemies(),
                                   gameWorld.getEnemyCount(),
                                   gameWorld.getBoss(),
                                   gameWorld.isBossSpawned(),
                                   bossDeathSlowMotionActive,
                                   bossDeathSlowMotionTimer,
                                   BOSS_DEATH_SLOWMO_DURATION,
                                   &keyBindingManager);
    combatSystem.updatePotions(dt, *gameWorld.getPlayer());
    combatSystem.updateMeteors(dt, *gameWorld.getPlayer(), gameWorld.getGroundLevel(), cameraController.getView(), rng, &gameMaster);

    // World/platforms and enemies
    gameWorld.updatePlatforms(dt);
    updateEnemies(dt);
    gameWorld.checkAndSpawnBoss();
    updateBoss(dt);

    // UI fluff and camera follow
    updateFloatingTexts(dt);
    cameraController.followPlayer(gameWorld.getPlayer());
}

// ---------- Update helpers ----------

void Session::updatePlayer(float dt)
{
    Player *player = gameWorld.getPlayer();

    // Movement input + jump handling
    player->handleInput(dt, &keyBindingManager);
    bool didJump = player->jump(dt, &keyBindingManager);
    if (didJump)
    {
        ResourceManager::getInstance().playJumpSound();
    }

    // For landing sound detection
    bool wasInAir = !player->getOnGround();

    // Physics + animation updates
    physicsManager.updatePlayerPhysics(*player, dt, gameWorld.getGroundLevel(), gameWorld.getPlatformRawArray(), static_cast<int>(gameWorld.getPlatformRawCount()));
    player->updateAnimation(dt);

    // Landed this frame?
    if (wasInAir && player->getOnGround())
    {
        ResourceManager::getInstance().playLandSound();
    }

    // Footstep audio while moving on ground and not attacking
    bool leftPressedNow = sf::Keyboard::isKeyPressed(keyBindingManager.getKeyForAction(GameAction::MOVE_LEFT));
    bool rightPressedNow = sf::Keyboard::isKeyPressed(keyBindingManager.getKeyForAction(GameAction::MOVE_RIGHT));
    bool isMoving = (leftPressedNow || rightPressedNow) && player->getOnGround() &&
                    !player->getIsAttacking() && !player->getIsSpecialAttacking();

    if (isMoving)
    {
        ResourceManager::getInstance().playWalkSound();
    }
    else
    {
        ResourceManager::getInstance().stopWalkSound();
    }

    // Clamp position horizontally within world bounds (considering sprite frame width)
    sf::Vector2f playerPosition = player->getPosition();
    const Animation *playerAnim = player->getCurrentAnimation();
    const float playerFrameWidth = playerAnim ? static_cast<float>(playerAnim->frameSize.x) : 0.f;
    playerPosition.x = std::clamp(playerPosition.x, -WORLD_MARGIN, WORLD_WIDTH + WORLD_MARGIN - playerFrameWidth);
    player->setPosition(playerPosition);

    // Update combat state (block, attack, cooldowns, etc.)
    combatSystem.updatePlayerCombat(dt, *player, bossDeathSlowMotionActive ? SLOWMO_TIME_SCALE : 1.0f, &keyBindingManager);
}

void Session::updateEnemies(float dt)
{
    Player *player = gameWorld.getPlayer();
    auto &enemies = gameWorld.getEnemies();
    std::size_t enemiesCount = gameWorld.getEnemyCount();
    auto &platforms = gameWorld.getPlatforms();
    std::size_t platformsCount = gameWorld.getPlatformCount();
    float groundLevel = gameWorld.getGroundLevel();

    const sf::FloatRect playerCollision = player->getCollisionBounds();

    for (std::size_t i = 0; i < enemiesCount; ++i)
    {
        auto &enemy = enemies[i];
        if (!enemy || !enemy->isAlive())
        {
            enemyAttackDamageApplied[i] = false;
            continue;
        }

        // AI + internal timers
        enemy->update(dt, GRAVITY, *player);

        // Physics collision against ground/platforms
        physicsManager.updateEnemyPhysics(*enemy, dt, groundLevel, gameWorld.getPlatformRawArray(), static_cast<int>(platformsCount));

        sf::FloatRect enemyBounds = enemy->getCollisionBounds();

        // Handle melee attack impact frames vs player
        if (enemy->getIsAttacking())
        {
            int currentAttackFrame = enemy->getAttackFrame();
            bool isImpactFrame = (currentAttackFrame >= 2 && currentAttackFrame <= 3);

            const sf::FloatRect attackBounds = enemy->getAttackBounds();
            if (isImpactFrame && !enemyAttackDamageApplied[i] && checkRectIntersection(attackBounds, playerCollision))
            {
                const int damage = enemy->getDamage();
                player->takeDamage(damage);
                enemyAttackDamageApplied[i] = true;

                ResourceManager::getInstance().playGotHitSound();
                spawnFloatingText(damage, sf::Vector2f(playerCollision.left + playerCollision.width / 2.f, playerCollision.top), sf::Color::Red);
                addCombatLog("Enemy attacked! Took " + std::to_string(damage) + " damage");
            }

            if (!isImpactFrame)
            {
                enemyAttackDamageApplied[i] = false;
            }
        }
        else
        {
            enemyAttackDamageApplied[i] = false;
        }

        // DOT / burn effect tick + feedback text
        enemy->updateBurnEffect(dt, [this](int damage, sf::Vector2f position, const sf::Color &color, bool isHealing)
                                { spawnFloatingText(damage, position, color, isHealing); });
    }
}

void Session::updateBoss(float dt)
{
    Boss *boss = gameWorld.getBoss();
    Player *player = gameWorld.getPlayer();
    bool bossSpawned = gameWorld.isBossSpawned();
    auto &platforms = gameWorld.getPlatforms();
    std::size_t platformsCount = gameWorld.getPlatformCount();
    float groundLevel = gameWorld.getGroundLevel();

    // No boss logic when not spawned or dead
    if (!bossSpawned || !boss->isAlive())
        return;

    const sf::FloatRect playerCollision = player->getCollisionBounds();

    // AI update (supports rage mode)
    boss->update(dt, GRAVITY, *player, gameMaster.isBossRageModeActive());

    // Physics resolution
    physicsManager.updateBossPhysics(*boss, dt, groundLevel, gameWorld.getPlatformRawArray(), static_cast<int>(platformsCount));

    sf::FloatRect bossBounds = boss->getCollisionBounds();

    // Handle boss melee impact frames vs player
    if (boss->getIsAttacking())
    {
        int currentAttackFrame = boss->getAttackFrame();
        bool isImpactFrame = (currentAttackFrame >= 2 && currentAttackFrame <= 3);

        if (isImpactFrame && !bossAttackDamageApplied)
        {
            const sf::FloatRect attackBounds = boss->computeAttackBounds(playerCollision.left + playerCollision.width / 2.f > bossBounds.left + bossBounds.width / 2.f);
            if (checkRectIntersection(attackBounds, playerCollision))
            {
                const int damage = static_cast<int>(std::round(boss->getDamage() * gameMaster.getBossDamageMultiplier()));
                player->takeDamage(damage);
                bossAttackDamageApplied = true;

                ResourceManager::getInstance().playGotHitSound();
                spawnFloatingText(damage, sf::Vector2f(playerCollision.left + playerCollision.width / 2.f, playerCollision.top), sf::Color::Red);
                addCombatLog("BOSS attacked! Took " + std::to_string(damage) + " damage!");
            }
        }

        if (!isImpactFrame)
        {
            bossAttackDamageApplied = false;
        }
    }
    else
    {
        bossAttackDamageApplied = false;
    }

    // One-time debug flag when HP dips very low (kept for parity)
    static bool debugLogged = false;
    if (boss->getHp() <= 10 && !debugLogged)
    {
        debugLogged = true;
    }

    // Transition to slow-mo victory sequence on boss death
    if (!boss->isAlive() && !bossDeathSlowMotionActive)
    {
        bossDeathSlowMotionActive = true;
        bossDeathSlowMotionTimer = BOSS_DEATH_SLOWMO_DURATION;
        pushState(GameStateType::BossDeathSlowMotion);
        addCombatLog("*** BOSS DEFEATED! ***");
    }

    // Burn damage feedback
    boss->updateBurnEffect(dt, [this](int damage, sf::Vector2f position, const sf::Color &color, bool isHealing)
                           { spawnFloatingText(damage, position, color, isHealing); });
}

void Session::updateFloatingTexts(float dt)
{
    // Tick animations
    floatingTexts.forEach([dt](std::unique_ptr<FloatingText> &dt_ptr)
                          {
        if (dt_ptr)
        {
            dt_ptr->update(dt);
        } });

    // Remove expired entries from the front of the queue
    int removedCount = 0;
    while (!floatingTexts.isEmpty() && floatingTexts.front()->isExpired())
    {
        floatingTexts.dequeue();
        removedCount++;
    }
}

// ==============================
// Rendering
// ==============================

void Session::render()
{
    // Apply camera and draw world-level entities
    cameraController.applyToWindow(window);
    renderWorld();

    // Optional debug overlays
    if (debugMode)
    {
        renderDebugCollisions();
        renderKeyDisplay();
    }

    // HUD and dynamic status indicators
    uiSystem.renderHUD(window,
                       *gameWorld.getPlayer(),
                       combatSystem.getProjectileAmmo(),
                       combatSystem.getProjectileCooldown(),
                       combatSystem.getSpecialAttackAmmo(),
                       combatSystem.getSpecialAttackCooldown(),
                       combatSystem.isEffectivelyUsingFireProjectile(*gameWorld.getPlayer()),
                       gameMaster.isRageModeActive(),
                       gameMaster.isMeteorFuryActive(),
                       gameMaster.isBossRageModeActive());

    // Periodic low-HP attention
    uiSystem.renderLowHpWarning(window, lowHpWarningTimer, gameWorld.getPlayer());

    // GameMaster visual effects (e.g., rage/meter)
    gameMaster.render(window);

    // State-driven overlays/menus
    if (getCurrentState() == GameStateType::Paused)
    {
        uiSystem.renderPauseMenu(window, stateStack.top());
    }

    if (getCurrentState() == GameStateType::PlayerStatsScreen)
    {
        uiSystem.renderPlayerStatsScreen(window, gameWorld.getPlayer());
    }

    if (getCurrentState() == GameStateType::InventoryOnlyScreen)
    {
        uiSystem.renderInventoryOnlyScreen(window, gameWorld.getPlayer());
    }

    if (getCurrentState() == GameStateType::SkillTreeScreen)
    {
        uiSystem.renderSkillTreeScreen(window, gameWorld.getPlayer(), stateStack.top().selectedSkillIndex);
    }

    if (getCurrentState() == GameStateType::HelpScreen)
    {
        uiSystem.renderHelpScreen(window);
    }

    if (getCurrentState() == GameStateType::CombatLogScreen)
    {
        uiSystem.renderCombatLog(window, combatLog, combatLogCurrentNode, combatLogTraversalCount, combatLogDeleteCount);
    }

    if (getCurrentState() == GameStateType::SettingsMenu)
    {
        uiSystem.renderSettingsMenu(window, stateStack.top());
    }

    if (getCurrentState() == GameStateType::AudioSettings)
    {
        uiSystem.renderAudioSettings(window, stateStack.top());
    }

    if (getCurrentState() == GameStateType::ControlsMenu)
    {
        uiSystem.renderControlsMenu(window, stateStack.top(), keyBindingManager);
    }

    if (getCurrentState() == GameStateType::ConfirmQuitToMenu ||
        getCurrentState() == GameStateType::ConfirmRestart ||
        getCurrentState() == GameStateType::ConfirmOverwriteSave)
    {
        uiSystem.renderConfirmDialog(window, stateStack.top());
    }

    // Toast messages (e.g., "Saved!", "No slot")
    uiSystem.renderToast(window);

    if (debugMode)
    {
        renderStateStack();
    }
}

void Session::renderWorld()
{
    // Mid-gray background to contrast entities
    window.clear(sf::Color(128, 128, 128));

    // Aliases for containers and counts
    auto &floorTiles = gameWorld.getFloorTiles();
    std::size_t floorTileCount = gameWorld.getFloorTileCount();
    auto &platforms = gameWorld.getPlatforms();
    std::size_t platformsCount = gameWorld.getPlatformCount();
    auto &enemies = gameWorld.getEnemies();
    std::size_t enemiesCount = gameWorld.getEnemyCount();
    Boss *boss = gameWorld.getBoss();
    bool bossSpawned = gameWorld.isBossSpawned();
    Player *player = gameWorld.getPlayer();

    // Background/floor pass
    for (std::size_t i = 0; i < floorTileCount; ++i)
        window.draw(floorTiles[i]);

    // Platforms
    for (std::size_t i = 0; i < platformsCount; ++i)
    {
        if (platforms[i])
            uiSystem.renderPlatformEntity(window, *platforms[i]);
    }

    // Potions and meteors (world-attached entities)
    for (const auto &potion : combatSystem.getHPPotions())
    {
        if (potion)
            uiSystem.renderHPPotionEntity(window, *potion);
    }

    for (const auto &meteor : combatSystem.getMeteors())
    {
        if (meteor)
            uiSystem.renderMeteorEntity(window, *meteor);
    }

    // Fire projectiles
    for (auto it = combatSystem.getFireProjectiles().begin(); it != combatSystem.getFireProjectiles().end(); ++it)
    {
        auto &projectile = *it;
        if (projectile)
            uiSystem.renderFireProjectileEntity(window, *projectile);
    }

    // Ice projectiles
    for (auto it = combatSystem.getIceProjectiles().begin(); it != combatSystem.getIceProjectiles().end(); ++it)
    {
        auto &projectile = *it;
        if (projectile)
            uiSystem.renderIceProjectileEntity(window, *projectile);
    }

    // Enemies
    for (std::size_t i = 0; i < enemiesCount; ++i)
    {
        auto &enemy = enemies[i];
        if (enemy)
            uiSystem.renderEnemyEntity(window, *enemy);
    }

    // Boss
    if (bossSpawned && boss)
        uiSystem.renderBossEntity(window, *boss, gameMaster.isBossRageModeActive());

    // Player always last to render above enemies if overlapping
    if (player)
        uiSystem.renderPlayerEntity(window, *player, gameMaster.isRageModeActive());

    // Screen-space floating text
    floatingTexts.forEach([this](std::unique_ptr<FloatingText> &dt_ptr)
                          {
        if (dt_ptr)
        {
            dt_ptr->draw(window);
        } });
}

void Session::renderGameOver()
{
    // Center the camera over player's X so the end screen shows relevant context
    sf::Vector2f playerPos = gameWorld.getPlayer()->getPosition();
    float cameraX = playerPos.x - SCREEN_WIDTH / 2.f;
    cameraX = std::clamp(cameraX, 0.f, WORLD_WIDTH - SCREEN_WIDTH);

    cameraController.setPosition(sf::Vector2f(cameraX + SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f));
    cameraController.applyToWindow(window);

    // Play one-time end sound (victory/defeat)
    if (!gameOverSoundPlayed)
    {
        gameOverSoundPlayed = true;
        if (gameWon)
        {
            ResourceManager::getInstance().playVictorySound();
        }
        else
        {
            ResourceManager::getInstance().playFailSound();
        }
    }

    // Compose game over UI and stats
    uiSystem.renderGameOverScreen(window,
                                  gameWon,
                                  *gameWorld.getPlayer(),
                                  gameWorld.getEnemies(),
                                  gameWorld.getEnemyCount(),
                                  combatSystem.getFireProjectiles(),
                                  combatSystem.getHPPotions(),
                                  combatSystem.getMeteors(),
                                  gameWorld.getFloorTiles(),
                                  gameWorld.getFloorTileCount(),
                                  gameWorld.getPlatforms(),
                                  gameWorld.getPlatformCount());
}

// ---------- Debug rendering helpers ----------

void Session::renderDebugCollisions()
{
    DebugRenderer debugRenderer(window);

    // Player
    Player *player = gameWorld.getPlayer();
    if (player)
        debugRenderer.draw(*player);

    // Enemies
    auto &enemies = gameWorld.getEnemies();
    std::size_t enemiesCount = gameWorld.getEnemyCount();
    for (std::size_t i = 0; i < enemiesCount; ++i)
    {
        auto &enemy = enemies[i];
        if (enemy)
            debugRenderer.draw(*enemy);
    }

    // Boss
    Boss *boss = gameWorld.getBoss();
    if (boss)
        debugRenderer.draw(*boss);

    // Platforms
    auto &platforms = gameWorld.getPlatforms();
    std::size_t platformsCount = gameWorld.getPlatformCount();
    for (std::size_t i = 0; i < platformsCount; ++i)
    {
        auto &platform = platforms[i];
        if (platform)
            debugRenderer.draw(*platform);
    }

    // Potions
    for (const auto &potion : combatSystem.getHPPotions())
    {
        if (potion)
            debugRenderer.draw(*potion);
    }

    // Fire projectiles
    for (auto it = combatSystem.getFireProjectiles().begin(); it != combatSystem.getFireProjectiles().end(); ++it)
    {
        auto &projectile = *it;
        if (projectile)
            debugRenderer.draw(*projectile);
    }

    // Ice projectiles
    for (auto it = combatSystem.getIceProjectiles().begin(); it != combatSystem.getIceProjectiles().end(); ++it)
    {
        auto &projectile = *it;
        if (projectile)
            debugRenderer.draw(*projectile);
    }

    // Meteors
    for (const auto &meteor : combatSystem.getMeteors())
    {
        if (meteor)
            debugRenderer.draw(*meteor);
}
}

void Session::renderKeyDisplay()
{
    // Switch to screen-space view for UI overlay
    sf::View screenView(sf::FloatRect(0.f, 0.f, SCREEN_WIDTH, SCREEN_HEIGHT));
    window.setView(screenView);

    // Backdrop box
    sf::RectangleShape box;
    box.setSize(sf::Vector2f(120.f, 60.f));
    box.setPosition(SCREEN_WIDTH / 2.f - 60.f, SCREEN_HEIGHT - 80.f);
    box.setFillColor(sf::Color(0, 0, 0, 210));
    box.setOutlineColor(sf::Color(100, 200, 255));
    box.setOutlineThickness(3.f);
    window.draw(box);

    // Text showing last key pressed (or "None")
    sf::Text keyText;
    keyText.setFont(ResourceManager::getInstance().getFont());
    keyText.setString(lastPressedKeyName.empty() ? "None" : lastPressedKeyName);
    keyText.setCharacterSize(28);
    keyText.setFillColor(sf::Color::Yellow);
    keyText.setStyle(sf::Text::Bold);

    // Center the text in the box
    sf::FloatRect keyBounds = keyText.getLocalBounds();
    keyText.setOrigin(keyBounds.width / 2.f, keyBounds.height / 2.f + keyBounds.top);
    keyText.setPosition(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT - 50.f);
    window.draw(keyText);

    // Restore gameplay camera
    cameraController.applyToWindow(window);
}

void Session::renderStateStack()
{
    // Switch to screen-space view for UI overlay
    sf::View screenView(sf::FloatRect(0.f, 0.f, SCREEN_WIDTH, SCREEN_HEIGHT));
    window.setView(screenView);

    const sf::Font &font = ResourceManager::getInstance().getFont();

    // Panel layout constants
    const float panelX = SCREEN_WIDTH - 210.f;
    const float panelWidth = 200.f;
    const float panelHeight = 280.f;
    const float panelY = (SCREEN_HEIGHT - panelHeight) / 2.f;

    // Panel background
    sf::RectangleShape panelBg(sf::Vector2f(panelWidth, panelHeight));
    panelBg.setPosition(panelX, panelY);
    panelBg.setFillColor(sf::Color(20, 30, 50, 220));
    panelBg.setOutlineColor(sf::Color(100, 180, 255));
    panelBg.setOutlineThickness(2.f);
    window.draw(panelBg);

    // Top bar
    sf::RectangleShape topBar(sf::Vector2f(panelWidth, 35.f));
    topBar.setPosition(panelX, panelY);
    topBar.setFillColor(sf::Color(50, 100, 180, 200));
    window.draw(topBar);

    // Title
    sf::Text titleText;
    titleText.setFont(font);
    titleText.setString("STATE STACK");
    titleText.setCharacterSize(16);
    titleText.setFillColor(sf::Color(255, 255, 255));
    titleText.setStyle(sf::Text::Bold);
    titleText.setPosition(panelX + 15.f, panelY + 8.f);
    window.draw(titleText);

    // Helper to map state -> label/color
    auto getStateInfo = [](GameStateType type) -> std::pair<std::string, sf::Color>
    {
        switch (type)
        {
        case GameStateType::MainMenu:
            return {"MainMenu", sf::Color(100, 255, 100)};
        case GameStateType::Playing:
            return {"Playing", sf::Color(100, 200, 255)};
        case GameStateType::Paused:
            return {"Paused", sf::Color(255, 150, 100)};
        case GameStateType::SettingsMenu:
            return {"Settings", sf::Color(200, 150, 255)};
        case GameStateType::AudioSettings:
            return {"Audio", sf::Color(200, 150, 255)};
        case GameStateType::ControlsMenu:
            return {"Controls", sf::Color(200, 150, 255)};
        case GameStateType::InventoryOnlyScreen:
            return {"Inventory", sf::Color(200, 200, 100)};
        case GameStateType::PlayerStatsScreen:
            return {"Stats", sf::Color(200, 200, 100)};
        case GameStateType::SkillTreeScreen:
            return {"Skills", sf::Color(150, 255, 150)};
        case GameStateType::HelpScreen:
            return {"Help", sf::Color(200, 200, 100)};
        case GameStateType::CombatLogScreen:
            return {"Log", sf::Color(200, 200, 100)};
        case GameStateType::BossDeathSlowMotion:
            return {"Death", sf::Color(255, 100, 50)};
        case GameStateType::ConfirmRestart:
            return {"Restart", sf::Color(255, 200, 100)};
        case GameStateType::ConfirmQuitToMenu:
            return {"Quit", sf::Color(255, 200, 100)};
        default:
            return {"Unknown", sf::Color(150, 150, 150)};
        }
    };

    // List the top of the stack downwards
    float textY = panelY + 40.f;

    for (int i = 0; i < 8 && !stateStack.isEmpty(); ++i)
    {
        try
        {
            const GameStateData &state = stateStack.peek(i);
            auto [stateName, stateColor] = getStateInfo(state.type);

            sf::RectangleShape stateBox(sf::Vector2f(panelWidth - 10.f, 28.f));
            stateBox.setPosition(panelX + 5.f, textY + i * 32.f);

            if (i == 0)
            {
                // Highlight the active state
                stateBox.setFillColor(sf::Color(80, 150, 200, 200));
                stateBox.setOutlineColor(stateColor);
                stateBox.setOutlineThickness(2.f);
            }
            else
            {
                stateBox.setFillColor(sf::Color(40, 60, 90, 150));
                stateBox.setOutlineColor(sf::Color(60, 90, 120));
                stateBox.setOutlineThickness(1.f);
            }

            window.draw(stateBox);

            sf::Text stateText;
            stateText.setFont(font);
            stateText.setString(stateName);
            stateText.setCharacterSize(18);
            stateText.setFillColor(stateColor);

            if (i == 0)
            {
                stateText.setStyle(sf::Text::Bold);
            }

            sf::FloatRect textBounds = stateText.getLocalBounds();
            float boxCenterX = panelX + 5.f + (panelWidth - 10.f) / 2.f;
            stateText.setOrigin(textBounds.width / 2.f, 0.f);
            stateText.setPosition(boxCenterX, textY + i * 32.f + 5.f);
            window.draw(stateText);
        }
        catch (const std::exception &)
        {
            break; // If peek beyond size throws, stop rendering list
        }
    }

    // Restore world camera after screen-space drawing
    cameraController.applyToWindow(window);
}

// ==============================
// Combat Log + Floating Text Helpers
// ==============================

void Session::addCombatLog(const std::string &message)
{
    // Append new message
    combatLog.pushBack(message);

    // Maintain ring buffer size
    while (combatLog.size() > MAX_COMBAT_LOG_ENTRIES)
    {
        combatLog.popFront();
    }
}

void Session::clearCombatLog()
{
    // Remove all log entries
    combatLog.clear();
}

void Session::spawnFloatingText(int damage, sf::Vector2f position, const sf::Color &color, bool isHealing)
{
    // Create and enqueue a new floating text element
    auto newText = std::make_unique<FloatingText>(ResourceManager::getInstance().getFont(), damage, position, color, isHealing);
    floatingTexts.enqueue(std::move(newText));
}

void Session::clearFloatingTexts(const char *context)
{
    // Flush all floating texts from the queue
    while (!floatingTexts.isEmpty())
    {
        floatingTexts.dequeue();
    }
}

// ==============================
// State & Flow Control
// ==============================

void Session::pushState(GameStateType type)
{
    // Push a new state with the entry timestamp
    GameStateData newState(type);
    newState.timeEntered = deltaClock.getElapsedTime();
    stateStack.push(newState);
}

void Session::popState()
{
    // Pop if available
    if (!stateStack.isEmpty())
    {
        stateStack.pop();
    }
}

void Session::changeState(GameStateType type)
{
    // Replace current state with a new one
    if (!stateStack.isEmpty())
    {
        stateStack.pop();
    }
    pushState(type);
}

GameStateType Session::getCurrentState() const
{
    // Default to MainMenu if there's no state yet
    if (stateStack.isEmpty())
    {
        return GameStateType::MainMenu;
    }
    return stateStack.top().type;
}

bool Session::isGamePaused() const
{
    // Paused if top state indicates so
    if (stateStack.isEmpty())
    {
        return false;
    }
    return stateStack.top().pausesGameLogic;
}

bool Session::shouldExit() const
{
    // True when user requested exit
    return requestExit;
}

// ==============================
// End-Condition Checks
// ==============================

void Session::checkGameOver()
{
    // Player death -> defeat
    if (!gameWorld.getPlayer()->isAlive())
    {
        gameOver = true;

        ResourceManager::getInstance().stopWalkSound();
        ResourceManager::getInstance().stopMeteorSound();
        gameMaster.stopAllSounds();
    }

    // Boss defeated and not already in gameOver -> win
    if (gameWon && !gameOver)
    {
        gameOver = true;

        ResourceManager::getInstance().stopWalkSound();
        ResourceManager::getInstance().stopMeteorSound();
        gameMaster.stopAllSounds();
    }
}
