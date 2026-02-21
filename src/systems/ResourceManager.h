#pragma once

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>

#include "Animation.h"
#include "HashTable.h"

// ResourceManager handles loading and managing all game resources
class ResourceManager
{
public:
    ResourceManager();
    ~ResourceManager() = default;

    ResourceManager(const ResourceManager &) = delete;
    ResourceManager &operator=(const ResourceManager &) = delete;
    ResourceManager(ResourceManager &&) = delete;
    ResourceManager &operator=(ResourceManager &&) = delete;

    // Loads all game resources (textures, audio, etc.)
    bool loadAll();

    // Retrieves a texture by name
    sf::Texture &getTexture(const std::string &name);

    // Retrieves an animation by name
    Animation &getAnimation(const std::string &name);

    // Retrieves enemy animation
    EnemyAnimation &getEnemyAnim();

    // Retrieves boss animation
    BossAnimation &getBossAnim();

    // Returns the game font
    sf::Font &getFont();

    // Retrieves meteor textures
    void getMeteorTextures(sf::Texture *outTextures[5]);

    // Returns background music reference
    sf::Music &getBackgroundMusic();

    // Sets and gets volume levels
    void setMusicVolume(float volume);
    void setSFXVolume(float volume);
    float getMusicVolume() const;
    float getSFXVolume() const;

    // Plays and stops different sound effects
    void playWalkSound();
    void stopWalkSound();
    void playAttackSound(int attackType = 1, float pitch = 1.0f);
    void playLandSound();
    void playJumpSound();
    void playFireballSound();
    void playMeteorSound();
    void stopMeteorSound();
    void playGotHitSound();
    void playVictorySound();
    void playFailSound();

    // Returns whether resources are loaded
    bool isLoaded() const;

private:
    // Internal helper functions to load each type of resource
    bool loadFont();
    bool loadTextures();
    bool loadMeteorTextures();
    bool loadPlayerAnimations();
    bool loadEnemyAnimations();
    bool loadBossAnimation();
    bool loadAudio();

    // Logging helpers for hash table activity
    void logTextureInsertion(const std::string &name, size_t previousSize, size_t previousBucketCount);
    void logAnimationInsertion(const std::string &name, size_t previousSize, size_t previousBucketCount);
    void logTextureLookup(const std::string &name, long long lookupNs);
    void logAnimationLookup(const std::string &name, long long lookupNs);
    void logTableResize(const char *label, size_t previousBuckets, size_t newBuckets, float previousLoadFactor);
    void printHashTableStats() const;

    // Hash tables for textures and animations
    HashTable<std::string, std::unique_ptr<sf::Texture>> textureMap;
    HashTable<std::string, std::unique_ptr<Animation>> animationMap;

    // Font resource
    sf::Font font;

    // Animation resources for enemies and bosses
    EnemyAnimation enemyAnim;
    BossAnimation bossAnim;

    // Background music
    sf::Music backgroundMusic;

    // Volume levels for music and SFX
    float musicVolume;
    float sfxVolume;

    // Sound buffers and corresponding sound objects
    sf::SoundBuffer walkBuffer;
    sf::Sound walkSound;
    sf::SoundBuffer attack1Buffer;
    sf::Sound attack1Sound;
    sf::SoundBuffer attack2Buffer;
    sf::Sound attack2Sound;
    sf::SoundBuffer landBuffer;
    sf::Sound landSound;
    sf::SoundBuffer jumpBuffer;
    sf::Sound jumpSound;
    sf::SoundBuffer fireballBuffer;
    sf::Sound fireballSound;
    sf::SoundBuffer meteorBuffer;
    sf::Sound meteorSound;
    sf::SoundBuffer gotHitBuffer;
    sf::Sound gotHitSound;
    sf::SoundBuffer victoryBuffer;
    sf::Sound victorySound;
    sf::SoundBuffer failBuffer;
    sf::Sound failSound;

    // Flag to indicate if all resources were loaded successfully
    bool resourcesLoaded;
};
