#include "ResourceManager.h"
#include "Constants.h"
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace
{
    // Helper function to load a texture from file
    bool loadTexture(sf::Texture &texture, const char *path)
    {
        if (texture.loadFromFile(path))
            return true;

        std::cerr << "Failed to load texture: " << path << std::endl;
        return false;
    }

    // Helper function to load a standard animation
    bool loadAnimation(Animation &animation, const char *path, int frames, const char *label)
    {
        if (animation.load(path, frames))
            return true;

        std::cerr << "Failed to load " << label << ": " << path << std::endl;
        return false;
    }

    // Helper function to load a boss animation (multi-row spritesheet)
    bool loadAnimation(BossAnimation &animation, const char *path, int frames, int rows, const char *label)
    {
        if (animation.load(path, frames, rows))
            return true;

        std::cerr << "Failed to load " << label << ": " << path << std::endl;
        return false;
    }

    std::string formatValue(double value, int precision)
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(precision) << value;
        return oss.str();
    }
}

// Constructor initializes texture and animation maps and audio volumes
ResourceManager::ResourceManager()
    : textureMap(32), animationMap(16),
      resourcesLoaded(false), musicVolume(80.0f), sfxVolume(80.0f)
{
}

// Returns a singleton instance of ResourceManager
ResourceManager &ResourceManager::getInstance()
{
    static ResourceManager instance;
    return instance;
}

// Loads all game assets including fonts, textures, animations, and audio
bool ResourceManager::loadAll()
{
    std::cout << "Loading resources..." << std::endl;

    if (!loadFont() ||
        !loadTextures() ||
        !loadMeteorTextures() ||
        !loadPlayerAnimations() ||
        !loadEnemyAnimations() ||
        !loadBossAnimation() ||
        !loadAudio())
    {
        std::cerr << "Resource loading failed." << std::endl;
        return false;
    }

    resourcesLoaded = true;
    std::cout << "All resources loaded successfully!" << std::endl;
    printHashTableStats();
    return true;
}

// Loads the main font for the game
bool ResourceManager::loadFont()
{
    if (!font.loadFromFile("assets/Roboto-Regular.ttf"))
    {
        std::cerr << "Failed to load font: Roboto-Regular.ttf" << std::endl;
        return false;
    }
    return true;
}

// Loads general textures into the texture hash table
bool ResourceManager::loadTextures()
{
    // Lambda function for inserting textures into hash table
    auto loadAndInsert = [this](const std::string &name, const char *path) -> bool
    {
        auto texture = std::make_unique<sf::Texture>();
        if (!texture->loadFromFile(path))
        {
            std::cerr << "Failed to load texture: " << path << std::endl;
            return false;
        }
        size_t previousSize = textureMap.size();
        size_t previousBucketCount = textureMap.bucketCount();
        textureMap.insert(name, std::move(texture));
        logTextureInsertion(name, previousSize, previousBucketCount);
        return true;
    };

    // Load all required textures
    return loadAndInsert("floor", "assets/floor.png") &&
           loadAndInsert("platform", "assets/platform.png") &&
           loadAndInsert("fireball", "assets/fireball.png") &&
           loadAndInsert("ice_projectile", "assets/iceshard.png") &&
           loadAndInsert("hp_potion", "assets/hp-potion.png") &&
           loadAndInsert("banner", "assets/banner-horizontal.png") &&
           loadAndInsert("easy", "assets/easy.png") &&
           loadAndInsert("medium", "assets/medium.png") &&
           loadAndInsert("hard", "assets/hard.png") &&
           loadAndInsert("select", "assets/select.png");
}

// Loads textures for meteor effects
bool ResourceManager::loadMeteorTextures()
{
    const char *paths[] = {
        "assets/meteor-1.png",
        "assets/meteor-2.png",
        "assets/meteor-3.png",
        "assets/meteor-4.png",
        "assets/meteor-5.png"};

    // Loop through meteor images
    for (int i = 0; i < 5; ++i)
    {
        auto texture = std::make_unique<sf::Texture>();
        if (!texture->loadFromFile(paths[i]))
        {
            std::cerr << "Failed to load texture: " << paths[i] << std::endl;
            return false;
        }

        std::string name = "meteor_" + std::to_string(i + 1);
        size_t previousSize = textureMap.size();
        size_t previousBucketCount = textureMap.bucketCount();
        textureMap.insert(name, std::move(texture));
        logTextureInsertion(name, previousSize, previousBucketCount);
    }
    return true;
}

// Loads all player animation assets
bool ResourceManager::loadPlayerAnimations()
{
    // Lambda to load and insert player animations
    auto loadAndInsert = [this](const std::string &name, const char *path, int frames) -> bool
    {
        auto anim = std::make_unique<Animation>();
        if (!anim->load(path, frames))
        {
            std::cerr << "Failed to load player animation: " << path << std::endl;
            return false;
        }
        size_t previousSize = animationMap.size();
        size_t previousBucketCount = animationMap.bucketCount();
        animationMap.insert(name, std::move(anim));
        logAnimationInsertion(name, previousSize, previousBucketCount);
        return true;
    };

    // Load player animations
    return loadAndInsert("player_run", "assets/warrior-run.png", 6) &&
           loadAndInsert("player_idle", "assets/warrior-idle.png", 8) &&
           loadAndInsert("player_attack1", "assets/warrior-attack-1.png", 4) &&
           loadAndInsert("player_attack2", "assets/warrior-attack-2.png", 4) &&
           loadAndInsert("player_guard", "assets/warrior-guard.png", 6);
}

// Loads enemy animation assets
bool ResourceManager::loadEnemyAnimations()
{
    if (!enemyAnim.loadRun("assets/lancer-run.png", 6))
    {
        std::cerr << "Failed to load enemy run animation: assets/lancer-run.png" << std::endl;
        return false;
    }

    if (!enemyAnim.loadAttack("assets/lancer-attack.png", 3))
    {
        std::cerr << "Failed to load enemy attack animation: assets/lancer-attack.png" << std::endl;
        return false;
    }

    if (!enemyAnim.loadIdle("assets/lancer-idle.png", 12))
    {
        std::cerr << "Failed to load enemy idle animation: assets/lancer-idle.png" << std::endl;
        return false;
    }

    return true;
}

// Loads boss animation spritesheet
bool ResourceManager::loadBossAnimation()
{
    return loadAnimation(bossAnim, "assets/boss.png", 6, 6, "boss animation");
}

// Loads all background music and sound effects
bool ResourceManager::loadAudio()
{
    // Load and configure background music
    if (!backgroundMusic.openFromFile("assets/sounds/background.wav"))
    {
        std::cerr << "Failed to load background music: assets/sounds/background.wav" << std::endl;
        return false;
    }

    backgroundMusic.setLoop(true);
    backgroundMusic.setVolume(musicVolume);

    // Load walking sound
    if (!walkBuffer.loadFromFile("assets/sounds/walk.wav"))
    {
        std::cerr << "Warning: Failed to load walk sound: assets/sounds/walk.wav" << std::endl;
    }
    else
    {
        walkSound.setBuffer(walkBuffer);
        walkSound.setVolume(sfxVolume);
    }

    // Load attack sound 1
    if (!attack1Buffer.loadFromFile("assets/sounds/attack-1.wav"))
    {
        std::cerr << "Warning: Failed to load attack-1 sound: assets/sounds/attack-1.wav" << std::endl;
    }
    else
    {
        attack1Sound.setBuffer(attack1Buffer);
        attack1Sound.setVolume(sfxVolume);
    }

    // Load attack sound 2
    if (!attack2Buffer.loadFromFile("assets/sounds/attack-2.wav"))
    {
        std::cerr << "Warning: Failed to load attack-2 sound: assets/sounds/attack-2.wav" << std::endl;
    }
    else
    {
        attack2Sound.setBuffer(attack2Buffer);
        attack2Sound.setVolume(sfxVolume);
    }

    // Load land sound
    if (!landBuffer.loadFromFile("assets/sounds/land.wav"))
    {
        std::cerr << "Warning: Failed to load land sound: assets/sounds/land.wav" << std::endl;
    }
    else
    {
        landSound.setBuffer(landBuffer);
        landSound.setVolume(sfxVolume);
    }

    // Load jump sound
    if (!jumpBuffer.loadFromFile("assets/sounds/jump.wav"))
    {
        std::cerr << "Warning: Failed to load jump sound: assets/sounds/jump.wav" << std::endl;
    }
    else
    {
        jumpSound.setBuffer(jumpBuffer);
        jumpSound.setVolume(sfxVolume);
    }

    // Load fireball sound
    if (!fireballBuffer.loadFromFile("assets/sounds/fireball.wav"))
    {
        std::cerr << "Warning: Failed to load fireball sound: assets/sounds/fireball.wav" << std::endl;
    }
    else
    {
        fireballSound.setBuffer(fireballBuffer);
        fireballSound.setVolume(sfxVolume);
    }

    // Load meteor sound
    if (!meteorBuffer.loadFromFile("assets/sounds/meteor.wav"))
    {
        std::cerr << "Warning: Failed to load meteor sound: assets/sounds/meteor.wav" << std::endl;
    }
    else
    {
        meteorSound.setBuffer(meteorBuffer);
        meteorSound.setVolume(sfxVolume);
    }

    // Load got-hit sound
    if (!gotHitBuffer.loadFromFile("assets/sounds/got-hit.wav"))
    {
        std::cerr << "Warning: Failed to load got-hit sound: assets/sounds/got-hit.wav" << std::endl;
    }
    else
    {
        gotHitSound.setBuffer(gotHitBuffer);
        gotHitSound.setVolume(sfxVolume);
    }

    // Load victory sound
    if (!victoryBuffer.loadFromFile("assets/sounds/victory.wav"))
    {
        std::cerr << "Warning: Failed to load victory sound: assets/sounds/victory.wav" << std::endl;
    }
    else
    {
        victorySound.setBuffer(victoryBuffer);
        victorySound.setVolume(sfxVolume);
    }

    // Load fail sound
    if (!failBuffer.loadFromFile("assets/sounds/fail.wav"))
    {
        std::cerr << "Warning: Failed to load fail sound: assets/sounds/fail.wav" << std::endl;
    }
    else
    {
        failSound.setBuffer(failBuffer);
        failSound.setVolume(sfxVolume);
    }

    return true;
}

void ResourceManager::logTextureInsertion(const std::string &name, size_t previousSize, size_t previousBucketCount)
{
    if (!ENABLE_HASH_TABLE_STDOUT)
    {
        return;
    }

    if (textureMap.bucketCount() != previousBucketCount && previousBucketCount > 0)
    {
        float loadFactorBefore = static_cast<float>(previousSize + 1) / static_cast<float>(previousBucketCount);
        logTableResize("Texture Map", previousBucketCount, textureMap.bucketCount(), loadFactorBefore);
    }

    size_t bucketIndex = textureMap.bucketIndex(name);
    std::cout << "[OK] Loaded texture: " << name << " (bucket " << bucketIndex << ")" << std::endl;
}

void ResourceManager::logAnimationInsertion(const std::string &name, size_t previousSize, size_t previousBucketCount)
{
    if (!ENABLE_HASH_TABLE_STDOUT)
    {
        return;
    }

    if (animationMap.bucketCount() != previousBucketCount && previousBucketCount > 0)
    {
        float loadFactorBefore = static_cast<float>(previousSize + 1) / static_cast<float>(previousBucketCount);
        logTableResize("Animation Map", previousBucketCount, animationMap.bucketCount(), loadFactorBefore);
    }

    size_t bucketIndex = animationMap.bucketIndex(name);
    std::cout << "[OK] Loaded animation: " << name << " (bucket " << bucketIndex << ")" << std::endl;
}

void ResourceManager::logTableResize(const char *label, size_t previousBuckets, size_t newBuckets, float previousLoadFactor)
{
    if (!ENABLE_HASH_TABLE_STDOUT)
    {
        return;
    }

    std::cout << "[OK] Resizing table: " << previousBuckets << " -> " << newBuckets
              << " buckets (load factor: " << formatValue(previousLoadFactor, 2) << ")";
    if (label != nullptr)
    {
        std::cout << " [" << label << "]";
    }
    std::cout << std::endl;
}

void ResourceManager::logTextureLookup(const std::string &name, long long lookupNs)
{
    if (!ENABLE_HASH_TABLE_STDOUT)
    {
        return;
    }

    size_t bucketIndex = textureMap.bucketIndex(name);
    std::cout << "[HashTable] - Retrieved \"" << name << "\" texture (" << lookupNs
              << "ns lookup, bucket " << bucketIndex << ")" << std::endl;
}

void ResourceManager::logAnimationLookup(const std::string &name, long long lookupNs)
{
    if (!ENABLE_HASH_TABLE_STDOUT)
    {
        return;
    }

    size_t bucketIndex = animationMap.bucketIndex(name);
    std::cout << "[HashTable] - Retrieved \"" << name << "\" animation (" << lookupNs
              << "ns lookup, bucket " << bucketIndex << ")" << std::endl;
}

void ResourceManager::printHashTableStats() const
{
    if (!ENABLE_HASH_TABLE_STDOUT)
    {
        return;
    }

    std::cout << "\nHash Table Statistics:\n";
    std::cout << "  Texture Map: " << textureMap.size() << " items, " << textureMap.bucketCount()
              << " buckets (load: " << formatValue(textureMap.loadFactor(), 2) << ")\n";
    std::cout << "  Animation Map: " << animationMap.size() << " items, " << animationMap.bucketCount()
              << " buckets (load: " << formatValue(animationMap.loadFactor(), 2) << ")\n";

    size_t totalItems = textureMap.size() + animationMap.size();
    size_t totalNonEmpty = textureMap.nonEmptyBucketCount() + animationMap.nonEmptyBucketCount();
    double averageChain = (totalNonEmpty == 0)
                              ? 0.0
                              : static_cast<double>(totalItems) / static_cast<double>(totalNonEmpty);

    size_t textureLongest = textureMap.longestChainLength();
    size_t animationLongest = animationMap.longestChainLength();
    size_t longestChain = textureLongest > animationLongest ? textureLongest : animationLongest;
    const char *quality = (longestChain <= 3)   ? "excellent distribution"
                          : (longestChain <= 6) ? "good distribution"
                                                : "needs attention";

    std::cout << "  Average chain length: " << formatValue(averageChain, 1) << " nodes" << std::endl;
    std::cout << "  Longest chain: " << longestChain << " nodes (" << quality << ")" << std::endl;
}

// Gets a texture by its name from the hash table
sf::Texture &ResourceManager::getTexture(const std::string &name)
{
    auto start = std::chrono::steady_clock::now();
    try
    {
        std::unique_ptr<sf::Texture> &texture = textureMap.get(name);
        long long lookupNs =
            std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start).count();
        logTextureLookup(name, lookupNs);
        return *texture;
    }
    catch (const std::runtime_error &)
    {
        std::cerr << "ERROR: Texture '" << name << "' not found in hash table!" << std::endl;
        return *textureMap.get("floor");
    }
}

// Gets an animation by its name from the hash table
Animation &ResourceManager::getAnimation(const std::string &name)
{
    auto start = std::chrono::steady_clock::now();
    try
    {
        std::unique_ptr<Animation> &anim = animationMap.get(name);
        long long lookupNs =
            std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start).count();
        logAnimationLookup(name, lookupNs);
        return *anim;
    }
    catch (const std::runtime_error &)
    {
        std::cerr << "ERROR: Animation '" << name << "' not found in hash table!" << std::endl;
        return *animationMap.get("player_idle");
    }
}

// Returns reference to main font
sf::Font &ResourceManager::getFont() { return font; }

// Retrieves meteor textures for external use
void ResourceManager::getMeteorTextures(sf::Texture *outTextures[5])
{
    for (int i = 0; i < 5; ++i)
    {
        std::string name = "meteor_" + std::to_string(i + 1);
        outTextures[i] = &getTexture(name);
    }
}

// Returns reference to enemy animation
EnemyAnimation &ResourceManager::getEnemyAnim() { return enemyAnim; }

// Returns reference to boss animation
BossAnimation &ResourceManager::getBossAnim() { return bossAnim; }

// Returns background music object
sf::Music &ResourceManager::getBackgroundMusic() { return backgroundMusic; }

// Sets background music volume (clamped)
void ResourceManager::setMusicVolume(float volume)
{
    musicVolume = std::clamp(volume, 0.0f, 100.0f);
    backgroundMusic.setVolume(musicVolume);
}

// Sets sound effects volume (clamped)
void ResourceManager::setSFXVolume(float volume)
{
    sfxVolume = std::clamp(volume, 0.0f, 100.0f);
    walkSound.setVolume(sfxVolume);
    attack1Sound.setVolume(sfxVolume);
    attack2Sound.setVolume(sfxVolume);
    landSound.setVolume(sfxVolume);
    fireballSound.setVolume(sfxVolume);
    meteorSound.setVolume(sfxVolume);
    jumpSound.setVolume(sfxVolume);
    gotHitSound.setVolume(sfxVolume);
    failSound.setVolume(sfxVolume);
}

// Returns current music volume
float ResourceManager::getMusicVolume() const { return musicVolume; }

// Returns current sound effects volume
float ResourceManager::getSFXVolume() const { return sfxVolume; }

// Plays walking sound (looped)
void ResourceManager::playWalkSound()
{
    if (walkBuffer.getSampleCount() == 0)
        return;

    if (walkSound.getStatus() != sf::Sound::Playing)
    {
        walkSound.setLoop(true);
        walkSound.setVolume(sfxVolume);
        walkSound.play();
    }
}

// Stops walking sound
void ResourceManager::stopWalkSound()
{
    if (walkSound.getStatus() == sf::Sound::Playing)
    {
        walkSound.stop();
    }
}

// Plays attack sound depending on attack type
void ResourceManager::playAttackSound(int attackType, float pitch)
{
    if (attackType == 2)
    {
        if (attack2Buffer.getSampleCount() == 0)
            return;
        attack2Sound.setVolume(sfxVolume);
        attack2Sound.setPitch(pitch);
        attack2Sound.play();
    }
    else
    {
        if (attack1Buffer.getSampleCount() == 0)
            return;
        attack1Sound.setVolume(sfxVolume);
        attack1Sound.setPitch(pitch);
        attack1Sound.play();
    }
}

// Plays landing sound effect
void ResourceManager::playLandSound()
{
    if (landBuffer.getSampleCount() == 0)
        return;

    landSound.setVolume(sfxVolume);
    landSound.play();
}

// Plays jump sound
void ResourceManager::playJumpSound()
{
    if (jumpBuffer.getSampleCount() == 0)
        return;

    jumpSound.setVolume(sfxVolume);
    jumpSound.play();
}

// Plays fireball sound
void ResourceManager::playFireballSound()
{
    if (fireballBuffer.getSampleCount() == 0)
        return;

    fireballSound.setVolume(sfxVolume);
    fireballSound.play();
}

// Plays looping meteor sound
void ResourceManager::playMeteorSound()
{
    if (meteorBuffer.getSampleCount() == 0)
        return;

    if (meteorSound.getStatus() != sf::Sound::Playing)
    {
        meteorSound.setLoop(true);
        meteorSound.setVolume(sfxVolume * 0.7f);
        meteorSound.setPitch(0.30f);
        meteorSound.play();
    }
}

// Stops meteor sound effect
void ResourceManager::stopMeteorSound()
{
    if (meteorSound.getStatus() == sf::Sound::Playing)
    {
        meteorSound.stop();
    }
}

// Plays got-hit sound with higher pitch
void ResourceManager::playGotHitSound()
{
    if (gotHitBuffer.getSampleCount() == 0)
        return;

    if (gotHitSound.getStatus() == sf::Sound::Playing)
    {
        gotHitSound.stop();
    }
    gotHitSound.setVolume(sfxVolume * 1.2f);
    gotHitSound.setPitch(1.8f);
    gotHitSound.play();
}

// Plays victory sound
void ResourceManager::playVictorySound()
{
    if (victoryBuffer.getSampleCount() == 0)
        return;

    victorySound.setVolume(sfxVolume);
    victorySound.play();
}

// Plays failure sound
void ResourceManager::playFailSound()
{
    if (failBuffer.getSampleCount() == 0)
        return;

    failSound.setVolume(sfxVolume);
    failSound.play();
}

// Returns whether all resources have been successfully loaded
bool ResourceManager::isLoaded() const { return resourcesLoaded; }
