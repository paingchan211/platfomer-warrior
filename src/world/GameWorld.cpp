#include "GameWorld.h"
#include "ResourceManager.h"
#include "PolymorphismLogger.h"
#include <cmath>
#include <iostream>

// Constructor initializes reference to resource manager only
GameWorld::GameWorld(ResourceManager &resourceManager)
    : resourceManager(resourceManager)
{
}

// Initializes all game world components (floor, platforms, player, enemies, boss)
bool GameWorld::initialize()
{
    // Calculate ground level based on constants
    const float floorY = WORLD_HEIGHT - FLOOR_TILE_HEIGHT + FLOOR_Y_OFFSET;
    groundLevel = floorY + FLOOR_COLLISION_OFFSET;

    // Set up floor and platforms
    setupFloorTiles();
    setupPlatforms();

    // Define player starting position
    playerStartX = 0.f;
    playerStartY = groundLevel - CHARACTER_HEIGHT;

    // Define boss spawn position just beyond the playable space so it walks in from the right
    bossSpawnPosition = sf::Vector2f{WORLD_WIDTH + BOSS_SPAWN_OFFSET_X, groundLevel - 100.f};

    // Create all entities
    createPlayer();
    createEnemies();
    createBoss();

    return true;
}

// Resets the world by recreating entities
void GameWorld::reset()
{
    createPlayer();
    createEnemies();
    createBoss();

    bossSpawned = false;
    allEnemiesDefeated = false;
}

// Creates a tiled floor across the map
void GameWorld::setupFloorTiles()
{
    const float floorY = WORLD_HEIGHT - FLOOR_TILE_HEIGHT + FLOOR_Y_OFFSET;
    const float floorSpan = WORLD_WIDTH + BOSS_ENTRANCE_FLOOR_PADDING;
    const int requiredTiles = static_cast<int>(std::ceil(floorSpan / FLOOR_TILE_WIDTH));
    floorTileCount = static_cast<std::size_t>(requiredTiles);
    floorTiles = std::make_unique<sf::Sprite[]>(floorTileCount);

    // Initialize each floor tile sprite
    for (int i = 0; i < requiredTiles; ++i)
    {
        floorTiles[i] = sf::Sprite(resourceManager.getTexture("floor"));
        floorTiles[i].setPosition({i * FLOOR_TILE_WIDTH, floorY});
    }
}

// Creates platforms with hardcoded level layout
void GameWorld::setupPlatforms()
{
    // Get platform texture from resource manager
    const sf::Texture &platformTexture = resourceManager.getTexture("platform");

    // Define platform configuration for this level
    platformsCount = 10;
    platforms = std::make_unique<std::unique_ptr<Platform>[]>(platformsCount);

    // Create platforms at specific positions with various sizes
    platforms[0] = std::make_unique<Platform>(platformTexture, 600.f, 500.f);

    // Create two platforms at similar heights
    platforms[1] = std::make_unique<Platform>(platformTexture, 1200.f, 400.f, 100.f, 50.f);
    platforms[2] = std::make_unique<Platform>(platformTexture, 1390.f, 400.f, 100.f, 50.f);

    // Create three platforms in a row
    platforms[3] = std::make_unique<Platform>(platformTexture, 2400.f, 450.f, 120.f, 60.f);
    platforms[4] = std::make_unique<Platform>(platformTexture, 2590.f, 450.f, 120.f, 60.f);
    platforms[5] = std::make_unique<Platform>(platformTexture, 2780.f, 450.f, 120.f, 60.f);

    // Create single platform at higher position
    platforms[6] = std::make_unique<Platform>(platformTexture, 4200.f, 320.f, 150.f, 70.f);

    // Create two platforms at similar heights
    platforms[7] = std::make_unique<Platform>(platformTexture, 4800.f, 420.f, 110.f, 50.f);
    platforms[8] = std::make_unique<Platform>(platformTexture, 4990.f, 420.f, 110.f, 50.f);

    // Create final platform at high position
    platforms[9] = std::make_unique<Platform>(platformTexture, 5800.f, 300.f, 130.f, 65.f);

    // Prepare raw pointer array for platform collision checks
    platformRawCount = platformsCount;
    platformRaw = std::make_unique<Platform *[]>(platformRawCount);
    for (std::size_t j = 0; j < platformsCount; ++j)
    {
        platformRaw[j] = platforms[j].get();
    }
}

// Creates the player at the starting position
void GameWorld::createPlayer()
{
    player = std::make_unique<Player>(
        &resourceManager.getAnimation("player_run"),
        &resourceManager.getAnimation("player_idle"),
        &resourceManager.getAnimation("player_attack1"),
        &resourceManager.getAnimation("player_attack2"),
        playerStartX,
        playerStartY,
        CHARACTER_HEIGHT,
        GRAVITY,
        JUMP_STRENGTH);
}

// Creates all enemies with specific spawn and patrol positions
void GameWorld::createEnemies()
{
    enemiesCount = 10;
    enemies = std::make_unique<std::unique_ptr<Enemy>[]>(enemiesCount);

    const float playerCollisionOffsetY = player->getCollisionOffsetY();
    const float floorEnemyY = groundLevel - playerCollisionOffsetY - ENEMY_CHARACTER_HEIGHT;

    // Enemy spawn configuration data
    const EnemySpawn spawnData[10] = {
        {800.f, floorEnemyY, 500.f, 1100.f},
        {1300.f, platforms[1]->getCollisionBounds().top - ENEMY_CHARACTER_HEIGHT, 1200.f, 1580.f},
        {2600.f, platforms[3]->getCollisionBounds().top - ENEMY_CHARACTER_HEIGHT, 2400.f, 2970.f},
        {4250.f, platforms[6]->getCollisionBounds().top - ENEMY_CHARACTER_HEIGHT, 4200.f, 4400.f},
        {4900.f, platforms[7]->getCollisionBounds().top - ENEMY_CHARACTER_HEIGHT, 4800.f, 5180.f},
        {5000.f, floorEnemyY, 4800.f, 5200.f},
        {5300.f, floorEnemyY, 5100.f, 5500.f},
        {5600.f, floorEnemyY, 5400.f, 5800.f},
        {5850.f, floorEnemyY, 5650.f, 5950.f},
        {5950.f, floorEnemyY, 5800.f, 5999.f}};

    // Create enemies from spawn data
    for (std::size_t i = 0; i < enemiesCount; ++i)
    {
        const auto &spawn = spawnData[i];
        enemies[i] = std::make_unique<Enemy>(
            spawn.x,
            spawn.y,
            spawn.patrolLeft,
            spawn.patrolRight,
            resourceManager.getEnemyAnim());
    }
}

// Creates the boss entity but does not spawn it yet
void GameWorld::createBoss()
{
    boss = std::make_unique<Boss>(resourceManager.getBossAnim());
    bossSpawned = false;
}

// Updates all moving platforms in the world
void GameWorld::updatePlatforms(float dt)
{
    static int platformFrameCounter = 0;
    platformFrameCounter++;

    bool loggedDispatch = false;
    for (std::size_t i = 0; i < platformsCount; ++i)
    {
        if (platforms[i])
        {
            Entity *platformEntity = platforms[i].get();
            if (!loggedDispatch)
            {
                if (ENABLE_POLYMORPHISM_STDOUT && platformFrameCounter == 120)
                {
                    std::cout << "  - Platform[" << i << "]::update() called (moving platform)\n";
                }
                logPolymorphicDispatch(platformEntity, "Entity", "update(float)");
                loggedDispatch = true;
            }
            platformEntity->update(dt);
        }
    }
}

// Checks if all enemies are defeated and spawns the boss if true
bool GameWorld::checkAndSpawnBoss()
{
    if (bossSpawned)
    {
        return false;
    }

    bool allDef = true;
    for (std::size_t i = 0; i < enemiesCount; ++i)
    {
        if (enemies[i] && enemies[i]->isAlive())
        {
            allDef = false;
            break;
        }
    }

    allEnemiesDefeated = allDef;

    // Spawn the boss when all enemies are defeated
    if (allEnemiesDefeated)
    {
        boss->spawn(bossSpawnPosition);
        bossSpawned = true;
        return true;
    }

    return false;
}
