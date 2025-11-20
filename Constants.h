#pragma once

#include <string>

// Debug / instrumentation toggles
constexpr bool ENABLE_HASH_TABLE_STDOUT = false;         // Mirror HashTable insert/lookups/stats
constexpr bool ENABLE_SINGLY_LINKED_LIST_STDOUT = false; // Mirror combat log operations to std::cout when true
constexpr bool ENABLE_DOUBLY_LINKED_LIST_STDOUT = true;  // Mirror DoublyLinkedList pool/active actions
constexpr bool ENABLE_STACK_STDOUT = false;              // Mirror Stack operations to std::cout when true
constexpr bool ENABLE_QUEUE_STDOUT = false;              // Mirror Queue operations to std::cout when true
constexpr bool ENABLE_SINGLETON_STDOUT = false;          // Mirror Singleton creation/destruction to std::cout when true
constexpr bool ENABLE_ITERATOR_STDOUT = false;           // Mirror Iterator operations to std::cout when true
constexpr bool ENABLE_INHERITANCE_STDOUT = false;        // Mirror base/derived constructor activity
constexpr bool ENABLE_POLYMORPHISM_STDOUT = false;       // Mirror virtual dispatch usage to std::cout when true
constexpr int POLYMORPHISM_LOG_LIMIT = 9999;             // Max logs per polymorphic dispatch site (unlimited for full frame logging)

// Screen and world dimensions
constexpr float SCREEN_WIDTH = 1200.f;        // Width of the game screen in pixels
constexpr float SCREEN_HEIGHT = 800.f;        // Height of the game screen in pixels
constexpr float WORLD_WIDTH = 6000.f;         // Total width of the game world in pixels
constexpr float WORLD_HEIGHT = SCREEN_HEIGHT; // Total height of the game world in pixels
constexpr float WORLD_MARGIN = 50.f;          // Margin for world boundaries in pixels

// Floor and tile properties
constexpr float FLOOR_Y_OFFSET = 80.f;     // Vertical offset for floor placement
constexpr float FLOOR_TILE_WIDTH = 490.f;  // Width of a floor tile in pixels
constexpr float FLOOR_TILE_HEIGHT = 196.f; // Height of a floor tile in pixels

// Character properties
constexpr float CHARACTER_HEIGHT = 192.f;       // Height of player character in pixels
constexpr float ENEMY_CHARACTER_HEIGHT = 320.f; // Height of enemy character in pixels
constexpr float FLOOR_COLLISION_OFFSET = 30.f;  // Collision offset for floor detection
constexpr float GRAVITY = 800.f;                // Gravity acceleration in pixels per second squared
constexpr float JUMP_STRENGTH = -600.f;         // Initial upward velocity for jump in pixels per second

// Projectile system constants
constexpr std::size_t MAX_PROJECTILES = 10;     // Maximum number of active projectiles
constexpr int MAX_PROJECTILE_AMMO = 3;          // Maximum projectile ammo count
constexpr float PROJECTILE_COOLDOWN_TIME = 5.f; // Cooldown time between projectile shots in seconds

// Special attack system constants
constexpr int MAX_SPECIAL_ATTACK_AMMO = 1;          // Maximum special attack ammo count
constexpr float SPECIAL_ATTACK_COOLDOWN_TIME = 5.f; // Cooldown time for special attack in seconds

// Item and effect limits
constexpr std::size_t MAX_HP_POTIONS = 20; // Maximum number of HP potions in the world
constexpr std::size_t MAX_METEORS = 20;    // Maximum number of meteors in the world

// Player starting stats and properties
constexpr int PLAYER_STARTING_HP = 100;               // Starting health points for player
constexpr int PLAYER_STARTING_DAMAGE = 25;            // Starting damage value for player
constexpr float PLAYER_MOVEMENT_SPEED = 300.f;        // Base movement speed for player in pixels per second
constexpr float PLAYER_SPEED_BOOST_MULTIPLIER = 1.5f; // Speed multiplier when player has speed boost
constexpr float PLAYER_HORIZONTAL_MARGIN = 55.f;      // Horizontal margin for player collision bounds
constexpr float PLAYER_GROUND_ADJUSTMENT = 8.f;       // Adjustment value for ground collision detection
constexpr int PLAYER_MAX_JUMPS = 2;                   // Maximum number of jumps the player can perform
constexpr float PLAYER_SPECIAL_DASH_SPEED = 600.f;    // Speed of special dash ability in pixels per second
constexpr float PLAYER_SPECIAL_DASH_DISTANCE = 200.f; // Distance covered by special dash in pixels

// Leveling system constants
constexpr int STARTING_LEVEL = 1;             // Starting level for player
constexpr int STARTING_EXPERIENCE = 0;        // Starting experience points
constexpr int BASE_EXP_TO_NEXT_LEVEL = 100;   // Base experience required to level up
constexpr int EXP_MULTIPLIER_PER_LEVEL = 100; // Additional experience required per level
constexpr int HP_GAIN_PER_LEVEL = 20;         // Health points gained per level
constexpr int DAMAGE_GAIN_PER_LEVEL = 5;      // Damage points gained per level

// Enemy starting stats and properties
constexpr int ENEMY_STARTING_HP = 75;              // Starting health points for enemies
constexpr int ENEMY_STARTING_DAMAGE = 15;          // Starting damage value for enemies
constexpr float ENEMY_MOVEMENT_SPEED = 100.f;      // Base movement speed for enemies in pixels per second
constexpr float ENEMY_CHASE_SPEED = 150.f;         // Chase speed for enemies in pixels per second
constexpr float ENEMY_DETECTION_RANGE = 300.f;     // Range at which enemies detect the player in pixels
constexpr float ENEMY_ATTACK_RANGE = 150.f;        // Attack range for enemies in pixels
constexpr float ENEMY_ANIMATION_FRAME_TIME = 0.1f; // Time per animation frame for enemies in seconds

// Boss starting stats and properties
constexpr int BOSS_STARTING_HP = 400;              // Starting health points for boss
constexpr int BOSS_STARTING_DAMAGE = 30;           // Starting damage value for boss
constexpr float BOSS_MOVEMENT_SPEED = 80.f;        // Base movement speed for boss in pixels per second
constexpr float BOSS_CHASE_SPEED = 120.f;          // Chase speed for boss in pixels per second
constexpr float BOSS_DETECTION_RANGE = 500.f;      // Range at which boss detects the player in pixels
constexpr float BOSS_ATTACK_RANGE = 120.f;         // Attack range for boss in pixels
constexpr float BOSS_SPRITE_SCALE = 2.5f;          // Scale multiplier for boss sprite
constexpr float BOSS_DEATH_SLOWMO_DURATION = 3.0f; // Duration of slow motion effect when boss dies in seconds
constexpr float SLOWMO_TIME_SCALE = 0.3f;          // Time scale multiplier for slow motion (0.3 = 30% speed)

// Status effect constants
constexpr float ICE_STACK_LIFETIME = 3.0f; // Lifetime of ice stack effect in seconds
constexpr int FREEZE_THRESHOLD = 2;        // Number of ice stacks required to freeze an enemy
constexpr float BURN_TICK_INTERVAL = 1.0f; // Interval between burn damage ticks in seconds

// UI and visual effect constants
constexpr float LOW_HP_FLASH_SPEED = 1.5f; // Speed of low HP flash effect
constexpr float LOW_HP_THRESHOLD = 0.40f;  // Health percentage threshold for low HP warning (40%)

// Player rage mode constants
constexpr float RAGE_HP_THRESHOLD = 0.30f;      // Health percentage threshold to activate rage (30%)
constexpr int RAGE_MIN_ENEMIES = 2;             // Minimum number of enemies required for rage activation
constexpr float RAGE_DETECTION_RANGE = 400.0f;  // Detection range for rage activation in pixels
constexpr float RAGE_DURATION = 8.0f;           // Duration of rage mode in seconds
constexpr float RAGE_COOLDOWN = 15.0f;          // Cooldown time for rage mode in seconds
constexpr float RAGE_DAMAGE_BONUS = 1.0f;       // Damage bonus multiplier during rage mode
constexpr float RAGE_OVERLAY_MAX_ALPHA = 60.0f; // Maximum alpha value for rage overlay effect
constexpr float RAGE_PULSE_SPEED = 3.0f;        // Speed of rage pulse effect

// Meteor system constants
constexpr float METEOR_FURY_MIN_INTERVAL = 1.0f;    // Minimum interval between meteors during fury mode in seconds
constexpr float METEOR_FURY_MAX_INTERVAL = 2.0f;    // Maximum interval between meteors during fury mode in seconds
constexpr float METEOR_NORMAL_MIN_INTERVAL = 10.0f; // Minimum interval between meteors in normal mode in seconds
constexpr float METEOR_NORMAL_MAX_INTERVAL = 20.0f; // Maximum interval between meteors in normal mode in seconds

// Boss rage mode constants
constexpr float BOSS_RAGE_HP_THRESHOLD = 0.50f;           // Health percentage threshold for boss rage activation (50%)
constexpr float BOSS_RAGE_DAMAGE_BONUS = 1.0f;            // Damage bonus multiplier for boss during rage
constexpr float BOSS_RAGE_SPEED_MULTIPLIER = 1.5f;        // Speed multiplier for boss during rage
constexpr float BOSS_RAGE_ATTACK_SPEED_MULTIPLIER = 1.5f; // Attack speed multiplier for boss during rage

// Structure for enemy spawn information
struct EnemySpawn
{
    float x;           // X position of spawn point
    float y;           // Y position of spawn point
    float patrolLeft;  // Left boundary of patrol area
    float patrolRight; // Right boundary of patrol area
};
