#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include "Player.h"
#include "Enemy.h"
#include "Boss.h"
#include "Platform.h"
#include "Constants.h"

// Forward declaration
class ResourceManager;

// Represents the main game world containing all entities and map components
class GameWorld
{
public:
    GameWorld(ResourceManager &resourceManager); // Constructor

    bool initialize();              // Sets up all game elements
    void reset();                   // Resets the world to initial state
    void updatePlatforms(float dt); // Updates moving platforms
    bool checkAndSpawnBoss();       // Spawns the boss when conditions are met

    // Player accessors
    Player *getPlayer() { return player.get(); }
    const Player *getPlayer() const { return player.get(); }

    // Boss accessors
    Boss *getBoss() { return boss.get(); }
    const Boss *getBoss() const { return boss.get(); }

    // Enemy accessors
    std::unique_ptr<std::unique_ptr<Enemy>[]> &getEnemies() { return enemies; }
    const std::unique_ptr<std::unique_ptr<Enemy>[]> &getEnemies() const { return enemies; }
    std::size_t getEnemyCount() const { return enemiesCount; }

    // Platform accessors
    std::unique_ptr<std::unique_ptr<Platform>[]> &getPlatforms() { return platforms; }
    const std::unique_ptr<std::unique_ptr<Platform>[]> &getPlatforms() const { return platforms; }
    std::size_t getPlatformCount() const { return platformsCount; }

    // Raw platform pointer array for collision detection
    Platform **getPlatformRawArray() { return platformRaw.get(); }
    const Platform *const *getPlatformRawArray() const { return platformRaw.get(); }
    std::size_t getPlatformRawCount() const { return platformRawCount; }

    // Floor tile accessors
    std::unique_ptr<sf::Sprite[]> &getFloorTiles() { return floorTiles; }
    const std::unique_ptr<sf::Sprite[]> &getFloorTiles() const { return floorTiles; }
    std::size_t getFloorTileCount() const { return floorTileCount; }

    // General world info
    float getGroundLevel() const { return groundLevel; }
    bool isBossSpawned() const { return bossSpawned; }
    bool areAllEnemiesDefeated() const { return allEnemiesDefeated; }

    // Starting and spawn positions
    sf::Vector2f getPlayerStartPosition() const { return sf::Vector2f(playerStartX, playerStartY); }
    sf::Vector2f getBossSpawnPosition() const { return bossSpawnPosition; }

    // Internal setup helpers
    void setupFloorTiles();
    void setupPlatforms();
    void createPlayer();
    void createEnemies();
    void createBoss();

    // Resource reference only
    ResourceManager &resourceManager;

    // Game entities
    std::unique_ptr<Player> player;
    std::unique_ptr<std::unique_ptr<Enemy>[]> enemies;
    std::size_t enemiesCount{0};
    std::unique_ptr<Boss> boss;

    // Platforms
    std::unique_ptr<std::unique_ptr<Platform>[]> platforms;
    std::size_t platformsCount{0};
    std::unique_ptr<Platform *[]> platformRaw;
    std::size_t platformRawCount{0};

    // Floor tiles
    std::unique_ptr<sf::Sprite[]> floorTiles;
    std::size_t floorTileCount{0};

    // World properties
    float groundLevel{0.f};
    float playerStartX{0.f};
    float playerStartY{0.f};
    sf::Vector2f bossSpawnPosition;
    bool bossSpawned{false};
    bool allEnemiesDefeated{false};
};
