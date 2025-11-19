## 1.2 Polymorphism

### 1.2.1 Application

**Application Area:** World entities, characters, and projectiles in the platformer (player, enemies, boss, platforms, potions, and different projectile types).

In this platformer, many different gameplay objects must be updated, moved, and checked for collisions every frame, even though their concrete behavior is very different. Examples include:

- The **player** reacting to input, jumping, attacking, and leveling up.
- **Enemies** patrolling, chasing, attacking, and reacting to status effects.
- The **boss** running its own state machine (spawn, idle, chase, attack, regenerate).
- **Platforms** (static and moving) that the player and enemies can stand on.
- **Projectiles** (fire and ice) that travel through the world and apply burn or freeze/slow effects.
- **Meteors** spawned by GameMaster events, falling under projectile handling (movement, collision, player damage).
- **Health potions** that float, animate, and can be collected to heal the player.

From the class hierarchy:

- `Entity` is the abstract base with `virtual void update(float dt) = 0;` and `virtual sf::FloatRect getBounds() const;`.
  - `Character` (player/enemy/boss) inherits from `Entity`.
  - `Platform`, `Projectile`, and `HPPotion` also inherit from `Entity`.
- `Projectile` is a (conceptually) abstract base for different projectile behaviors.
  - `FireProjectile` and `IceProjectile` both inherit from `Projectile` and override `update(float dt)` and `getBounds() const`.

**Desired Operation (Detailed):**

- The **game loop** and **physics/collision systems** should be able to:
  - Call `update(dt)` each frame on a set of objects without needing to know whether each object is a `Player`, `Enemy`, `Boss`, `Platform`, `Projectile`, or `HPPotion`.
  - Call `getBounds()` on any entity to perform collision checks (e.g., player vs. platform, projectile vs. enemy, player vs. potion).
- The **combat system** should:
  - Treat all projectiles through their common interface: moving them, checking collisions via `getBounds()`, and applying damage/effects (burn, freeze/slow) depending on the concrete projectile type.
  - Allow future projectiles (e.g., lightning, poison) to plug into the same update/collision logic by deriving from `Projectile` and overriding the virtual functions, without reworking the whole combat system.
- The **rendering/debug systems** (e.g., `DebugRenderer`) should:
  - Be able to inspect or draw different entity types using consistent interfaces, while still being able to call specialized methods where appropriate (e.g., projectile-specific debug drawing, enemy state visualization).

Polymorphism is used here so that the world, physics, and combat code can work with references or pointers to base classes (`Entity`, `Character`, `Projectile`) and rely on virtual functions to dispatch to the correct derived behavior at runtime.

### 1.2.2 Concept

**How Polymorphism Matches the Desired Operation:**

- **Common Interfaces, Different Behaviors:**

  - `Entity::update(float dt)` and `Entity::getBounds() const` define a common interface for all in-world entities.
  - `Character`, `Player`, `Enemy`, `Boss`, `Platform`, `Projectile`, `FireProjectile`, `IceProjectile`, and `HPPotion` all implement or override these methods to provide type-specific behavior.
  - The game loop and physics code can therefore manipulate a collection of `Entity*` / `Entity&` and call `update(dt)` and `getBounds()` without any `if`/`switch` on concrete types.

- **Specialized Character Logic via Virtual Functions:**

  - `Character::takeDamage(int)` and `Character::isAlive() const` provide shared combat behavior for all characters.
  - `Boss` overrides `isAlive() const` (if needed) and `getBounds()` / `getCollisionBounds()` to reflect its larger size and special behavior, but the rest of the system can still treat it as a `Character`/`Entity`.
  - This allows combat and game-over logic to operate on `Character`/`Entity` references without caring whether they point to the player, an enemy, or the boss.

- **Projectile Polymorphism:**

  - `Projectile` provides a shared base with `initializeProjectile`, `updateLifetime`, `getDamage()`, and virtual `reset()` / `update()` / `getBounds()`.
  - `FireProjectile` and `IceProjectile` override `update(float dt)` and `getBounds() const`, and add their own data and methods:
    - `FireProjectile` adds `burnDamage` and `burnDuration` and animates using a different frame size and sprite sheet.
    - `IceProjectile` adds `freezeDuration` and `slowAmount`, plus its own animation details.
  - `Meteor` extends `Projectile` as well and overrides the same virtual surface (`reset`, `update`, `getBounds`) so falling rocks can share pooling, collision, and damage logic with other projectiles while still animating through their five-frame sprite loop.

- **Extensibility:**
  - Adding a new entity type (for example, a new projectile or a different pickup) only requires creating a new class derived from `Entity` or `Projectile` and implementing the virtual methods.
  - Existing systems that iterate over `Entity` or `Projectile` pointers automatically support the new type without any structural changes.

**Alternative Structures and Why They Are Less Preferred Here:**

1. **Manual Type Checking with Enums and `switch`:**

   - Approach:
     - Have a single `Entity`-like struct with a `type` enum (`Player`, `Enemy`, `Boss`, `Platform`, `Projectile`, `Potion`, etc.).
     - In `updateAllEntities()`, use `switch(type)` and run different code for each case.
   - Cons:
     - Every time a new type is added, all `switch` statements must be updated.
     - Logic becomes centralized in big `switch` blocks instead of being encapsulated in each class.
     - Harder to test and extend; violates the open/closed principle.

2. **Separate Non-Polymorphic Update Loops for Each Type:**

   - Approach:
     - Maintain separate arrays/vectors: `players`, `enemies`, `bosses`, `platforms`, `projectiles`, `potions`.
     - Call a different update function for each container.
   - Cons:
     - Duplicates iteration patterns and collision checks across many functions.
     - Makes it harder to write generic logic that should apply to “all entities” (e.g., camera culling, debug rendering).
   - Polymorphism advantage:
     - With a base `Entity` interface, many operations can be written once and reused for all derived types.

3. **Templates / `std::variant` Instead of Virtual Functions:**
   - Approach:
     - Use `std::variant<Player, Enemy, Boss, Platform, ...>` and visitors for behavior.
   - Cons:
     - Increases complexity and boilerplate (visitor functions, lambdas) for each operation.
     - Every time a new type is added, the variant and all visitors must be updated.
   - For this project:
     - Classic runtime polymorphism with virtual functions is simpler, more idiomatic for OO C++, and easier to explain in a report.

For this platformer, runtime polymorphism via virtual functions and base/derived classes is preferred because it:

- Matches the need to treat many different entities uniformly in update and collision code.
- Encapsulates type-specific behavior within each derived class.
- Keeps extension (adding new entity or projectile types) straightforward without rewriting existing loops.

### 1.2.3 Implementation & Output

**Code Implementation**

The polymorphic entity system is built on a hierarchy starting from `Entity` as the abstract base, with specialized classes extending it to support different gameplay behaviors.

---

**Entity.h – Abstract Base Class for All In-World Entities**

Purpose: Provides the minimal data and interface that every in-world actor (player, enemies, boss, platforms, projectiles, potions, etc.) shares.

```cpp
#pragma once
#include <SFML/Graphics.hpp>

// Base entity class that provides common functionality for all game entities
class Entity
{
protected:
    sf::Sprite sprite;       // Sprite for rendering the entity
    sf::Vector2f position;   // Position of the entity in the world
    sf::Vector2f velocity;   // Velocity of the entity
    bool active;             // Whether the entity is currently active

public:
    // Default constructor that initializes entity with default values
    Entity();
    // Virtual destructor for proper inheritance
    virtual ~Entity() = default;

    // Pure virtual update method that must be implemented by derived classes
    virtual void update(float dt) = 0;
    // Returns the global bounds of the entity sprite
    virtual sf::FloatRect getBounds() const;

    // Returns the position of the entity
    sf::Vector2f getPosition() const;
    // Returns the velocity of the entity
    sf::Vector2f getVelocity() const;
    // Returns whether the entity is active
    bool isActive() const;
    // Returns a const reference to the entity's sprite
    const sf::Sprite &getSprite() const;

    // Sets the position of the entity
    void setPosition(sf::Vector2f pos);
    // Sets the velocity of the entity
    void setVelocity(sf::Vector2f vel);
    // Sets whether the entity is active
    void setActive(bool isActive);
};
```

---

**Character.h – Base Class for Combat-Capable Entities**

Purpose: Extends Entity with health, damage, movement speed, and "on ground" state used by all combat characters (Player, Enemy, Boss).

```cpp
#pragma once
#include "Entity.h"

// Base character class that provides combat stats and common character functionality
class Character : public Entity
{
protected:
    int hp;          // Current health points
    int maxHp;       // Maximum health points
    int damage;      // Damage dealt by this character
    bool onGround;   // Whether the character is currently on the ground
    float speed;     // Movement speed of the character

public:
    // Default constructor that initializes character with default values
    Character();
    // Virtual destructor for proper inheritance
    virtual ~Character() = default;

    // Applies damage to the character
    virtual void takeDamage(int dmg);
    // Checks if the character is still alive (HP > 0)
    virtual bool isAlive() const;

    // Returns the current health points
    int getHp() const;
    // Returns the maximum health points
    int getMaxHp() const;
    // Returns the damage value
    int getDamage() const;
    // Returns whether the character is on the ground
    bool getOnGround() const;
    // Returns the movement speed
    float getSpeed() const;

    // Sets the current health points (clamped to valid range)
    void setHp(int health);
    // Sets the maximum health points (must be non-negative)
    void setMaxHp(int maxHealth);
    // Sets the damage value (must be non-negative)
    void setDamage(int dmg);
    // Sets whether the character is on the ground
    void setOnGround(bool ground);
    // Sets the movement speed (must be non-negative)
    void setSpeed(float spd);

protected:
    // Initializes combat stats (HP and damage) for the character
    void initializeCombatStats(int initialHp, int initialDamage);
};
```

---

**Projectile.h – Base Class for Projectile-Type Entities**

Purpose: Extends Entity to provide shared projectile behavior (damage, lifetime tracking, reset interface) for all projectile types (FireProjectile, IceProjectile, Meteor).

```cpp
#pragma once
#include "Entity.h"

// Base projectile class (fireball, ice shard, meteor, etc.)
class Projectile : public Entity
{
protected:
    int damage;     // Amount of damage dealt
    float lifetime; // Time until projectile deactivates

public:
    Projectile();
    virtual ~Projectile() = default;

    // Reset projectile (implemented by derived classes)
    virtual void reset() = 0;

    // Accessors
    int getDamage() const;
    float getLifetime() const;

protected:
    // Initialize projectile attributes
    void initializeProjectile(int initialDamage, float initialLifetime);

    // Reduce lifetime over time
    void updateLifetime(float dt);
};
```

---

**FireProjectile.h – Fire Projectile Derived from Projectile**

Purpose: Specializes Projectile to represent a fire shot with animated frames and burn damage-over-time effects, reusing base projectile movement and lifetime logic.

```cpp
#pragma once

#include "Projectile.h"
#include "Animation.h"

// Fire projectile class that extends Projectile with burn effect capability
class FireProjectile : public Projectile
{
private:
    static constexpr int FRAME_WIDTH = 100;   // Width of each animation frame in pixels
    static constexpr int FRAME_HEIGHT = 100;  // Height of each animation frame in pixels
    static constexpr int TOTAL_FRAMES = 7;    // Total number of animation frames

    float animTime;      // Timer for animation frame updates
    int currentFrame;    // Current animation frame index
    int burnDamage;      // Damage per burn tick applied to enemies hit by this projectile
    float burnDuration;  // Duration of burn effect in seconds

public:
    // Constructor that initializes the fire projectile with texture, position, and direction
    FireProjectile(const sf::Texture &texture, const sf::Vector2f &startPos, bool facingRight);

    // Resets the projectile to default state (empty implementation)
    void reset() override;
    // Resets the projectile with new texture, position, and direction
    void reset(const sf::Texture &texture, const sf::Vector2f &startPos, bool facingRight);

    // Updates the projectile position, animation, and lifetime
    void update(float dt) override;
    // Returns the collision bounds of the projectile (smaller than visual bounds)
    sf::FloatRect getBounds() const override;

    // Returns the burn damage applied by this projectile
    int getBurnDamage() const;
    // Returns the burn duration applied by this projectile
    float getBurnDuration() const;
};
```

---

**IceProjectile.h – Ice Projectile Derived from Projectile**

Purpose: Specializes Projectile to represent an ice shot that animates and applies slow/freeze effects instead of burn, reusing the base projectile movement and lifetime logic.

```cpp
#pragma once

#include "Projectile.h"
#include "Animation.h"

// Represents an ice projectile that slows and freezes enemies
class IceProjectile : public Projectile
{
private:
    // Animation constants
    static constexpr int FRAME_WIDTH = 48;
    static constexpr int FRAME_HEIGHT = 32;
    static constexpr int TOTAL_FRAMES = 10;

    float animTime;       // Animation timer
    int currentFrame;     // Current frame index
    float freezeDuration; // Duration target is frozen
    float slowAmount;     // Movement slow multiplier

public:
    IceProjectile(const sf::Texture &texture, const sf::Vector2f &startPos, bool facingRight); // Constructor

    void reset() override;                                                                  // Default reset
    void reset(const sf::Texture &texture, const sf::Vector2f &startPos, bool facingRight); // Full reset

    void update(float dt) override;           // Updates position and animation
    sf::FloatRect getBounds() const override; // Returns collision bounds

    float getFreezeDuration() const; // Returns freeze duration
    float getSlowAmount() const;     // Returns slow effect
};
```

---

**Meteor.h – Meteor Derived from Projectile**

Purpose: Specializes Projectile to represent a falling meteor hazard with animated frames, reusing base projectile movement and lifetime logic, and designed for procedural environmental events (e.g., meteor showers) triggered by the GameMaster/AI Storyteller system.

```cpp
#pragma once

#include "Projectile.h"

// Meteor class inherits from Projectile and represents a falling meteor projectile
class Meteor : public Projectile
{
private:
    int currentFrame;         // Current animation frame index
    float animationTimer;     // Timer to control animation frame changes
    sf::Texture *textures[5]; // Array of texture frames for meteor animation

public:
    // Constructor initializes meteor with textures and a start position
    Meteor(sf::Texture *meteorTextures[5], sf::Vector2f startPos);

    // Reset functions to reuse meteor object
    void reset() override;
    void reset(sf::Texture *meteorTextures[5], sf::Vector2f startPos);

    // Update meteor movement and animation
    void update(float dt) override;

    // Return the bounding box for collision detection
    sf::FloatRect getBounds() const override;
};
```

---

**Polymorphism in Action: Usage Example**

The polymorphic entity system allows the game loop and combat system to treat all entities uniformly through base class pointers/references:

```cpp
// Example: Game loop updating all entities polymorphically
std::vector<Entity*> allEntities;

// Add different entity types to the vector
allEntities.push_back(player.get());           // Player* → Entity*
for (auto& enemy : enemies) {
    allEntities.push_back(enemy.get());        // Enemy* → Entity*
}
allEntities.push_back(boss.get());             // Boss* → Entity*
for (auto& platform : platforms) {
    allEntities.push_back(platform.get());     // Platform* → Entity*
}
for (auto& proj : fireProjectiles) {
    allEntities.push_back(proj.get());         // FireProjectile* → Entity*
}
for (auto& proj : iceProjectiles) {
    allEntities.push_back(proj.get());         // IceProjectile* → Entity*
}
for (auto& meteor : meteors) {
    allEntities.push_back(meteor.get());       // Meteor* → Entity*
}

// Update all entities with a single loop - virtual dispatch calls the correct update()
for (Entity* entity : allEntities) {
    if (entity && entity->isActive()) {
        entity->update(deltaTime);  // Polymorphic call - dispatches to correct derived class
    }
}

// Collision detection using polymorphic getBounds()
sf::FloatRect playerBounds = player->getBounds();  // Calls Player::getBounds() (or Character/Entity version)
for (auto& proj : fireProjectiles) {
    if (proj->isActive() && playerBounds.intersects(proj->getBounds())) {
        // Collision detected - proj->getBounds() calls FireProjectile::getBounds()
        handleProjectileCollision(player.get(), proj.get());
    }
}
```

**Output / Behavior Demonstration**

When the game runs, polymorphism enables the following runtime behaviors:

1. **Unified Update Loop:**

   - The main game loop calls `update(dt)` on an `Entity*` collection
   - Runtime dispatch ensures:
     - `Player::update()` handles input, jumping, attacking
     - `Enemy::update()` runs AI patrol/chase state machine
     - `Boss::update()` executes boss-specific behavior (spawn, idle, attack, regenerate)
     - `Platform::update()` moves platforms (if moving type)
     - `FireProjectile::update()` animates fire frames and moves horizontally
     - `IceProjectile::update()` animates ice frames and moves horizontally
     - `Meteor::update()` animates meteor frames and falls vertically

2. **Uniform Collision Checking:**

   - Physics system calls `getBounds()` on `Entity*` pointers
   - Each derived class returns appropriate collision rectangle:
     - Player/Enemy/Boss return character collision bounds (with margins)
     - FireProjectile returns fire sprite bounds
     - IceProjectile returns ice sprite bounds
     - Meteor returns meteor sprite bounds
   - All collision logic works generically without type checking

3. **Combat System Projectile Handling:**

   - Combat system stores `Projectile*` collections (fire, ice, meteors)
   - Calls `update()`, `getBounds()`, `getDamage()`, `reset()` polymorphically
   - Adding new projectile types (e.g., LightningProjectile) only requires:
     - Creating new class derived from `Projectile`
     - Overriding `update()`, `getBounds()`, `reset()`
     - No changes to combat system collision/damage logic

4. **Console Output Example (Debug Mode):**
   ```
   [Frame 120] Updating 15 entities
     - Player::update() called (input handling, jump physics)
     - Enemy[0]::update() called (AI state: CHASING)
     - Enemy[1]::update() called (AI state: PATROLLING)
     - Boss::update() called (state: ATTACKING)
     - Platform[0]::update() called (moving platform)
     - FireProjectile[0]::update() called (animating frame 3/7)
     - IceProjectile[0]::update() called (animating frame 5/10)
     - Meteor[0]::update() called (falling, frame 2/5)

   [Collision] Player bounds: (150, 450, 82, 192)
   [Collision] Checking projectile collisions...
     - FireProjectile[0] bounds: (400, 460, 80, 80) - No collision
     - IceProjectile[0] bounds: (180, 455, 40, 28) - COLLISION DETECTED!
       -> Applying ice slow effect to Player
   ```

**Visual Output:**

During gameplay, polymorphism manifests visually as:

- **Multiple entity types coexisting:** Player, enemies, boss, projectiles, meteors, platforms all active simultaneously
- **Type-specific animations:** Fire projectiles show flame animations, ice projectiles show frost animations, meteors cycle through rock frames
- **Correct collision responses:** Fire projectiles apply burn DoT, ice projectiles apply freeze/slow, meteors deal impact damage
- **Boss behavior variations:** Boss uses different attack patterns (inherited from Character but with overridden behavior)
- **GameMaster procedural events:** Meteor frequency increases during Meteor Fury Mode (GameMaster AI Storyteller event), all meteors update/animate correctly through polymorphic interface

The polymorphic design ensures that extending the game with new entity types (e.g., new enemies, new projectiles, new platforms) requires minimal changes to existing systems—only the new derived class needs to be implemented, and it automatically integrates with all entity management, collision, rendering, and update loops.

### 1.2.4 Troubleshooting Summary

**Issues Encountered During Implementation:**

**Issue 1: Ensuring Correct Virtual Dispatch (Avoiding Object Slicing)**

- **Problem:** There was a risk of storing derived objects (e.g., `Player`, `Enemy`, `Platform`, `FireProjectile`, `IceProjectile`) by value in containers of base type (e.g., `std::vector<Entity>` or `std::vector<Projectile>`), which would slice off the derived parts and break polymorphic behavior.
- **Root Cause:** In C++, copying a derived object into a container of base objects causes “object slicing,” removing derived fields and the dynamic type information needed for virtual calls.
- **Solution:** Ensured polymorphic objects are always managed via pointers or smart pointers (e.g., `std::unique_ptr<Player>`, `std::unique_ptr<Enemy>`, `std::unique_ptr<Platform>`, `std::unique_ptr<FireProjectile>`, `std::unique_ptr<IceProjectile>`) and referred to via `Entity*`, `Character*`, or `Projectile*` when calling virtual methods like `update(dt)` or `getBounds()`.

**Issue 2: Base Class Design for Polymorphism**

- **Problem:** Deciding which operations should be virtual in `Entity`, `Character`, and `Projectile` was not obvious at the start. Some behaviors were initially duplicated in derived classes or handled with manual type checks.
- **Root Cause:** The responsibilities of each base class (what is shared vs. what is specialized) needed clearer separation to support clean polymorphic use.
- **Solution:** Refined the interfaces:
  - Kept `update(float dt)` and `getBounds() const` virtual in `Entity` so that all entities can be updated and queried for collisions polymorphically.
  - Kept combat-specific virtual behavior (like `takeDamage` / `isAlive`) in `Character` and overridden in derived classes where necessary (e.g., `Boss`).
  - Kept projectile-specific behavior in `Projectile` and overridden in `FireProjectile` and `IceProjectile` while still calling shared helpers (`initializeProjectile`, `updateLifetime`).

**Issue 3: Overriding vs. Hiding Methods**

- **Problem:** There was potential confusion between overriding base virtual methods and accidentally hiding them (e.g., mismatched const-ness or parameter types in `update` / `getBounds` implementations).
- **Root Cause:** In C++, if a derived class method does not exactly match the base virtual signature, it may hide the base method instead of overriding it, leading to unexpected behavior when calling through a base pointer.
- **Solution:**
  - Carefully matched method signatures (including `const` and `override`) for functions like `update(float dt)` and `sf::FloatRect getBounds() const`.
  - Used the `override` keyword (as seen in `Boss`, `Player`, `Enemy`, `Platform`, `FireProjectile`, `IceProjectile`, `HPPotion`) to let the compiler catch mismatches.

**Issue 4: Managing Lifetime and Ownership of Polymorphic Objects**

- **Problem:** With many polymorphic objects (entities, projectiles, potions) being created and destroyed during gameplay, there was a risk of leaks or dangling pointers if lifetime was not managed carefully.
- **Root Cause:** Raw pointers to polymorphic types can easily become invalid if objects are deleted from pools or containers while other systems still hold references.
- **Solution:**
  - Used RAII and smart pointers (`std::unique_ptr<...>`) for owning polymorphic objects where appropriate (e.g., enemies stored as `std::unique_ptr<Enemy>`, projectiles in pooled `DoublyLinkedList<std::unique_ptr<FireProjectile>>`, etc.).
  - Ensured that systems accessing these objects either:
    - Work within the scope where the container is stable, or
    - Update or clear references when objects are removed from the pool.

**Resources / References Used:**

- C++ references and tutorials on:
  - Virtual functions and runtime polymorphism.
  - The `override` keyword and how it helps prevent signature mismatches.
  - Object slicing and why polymorphic objects should be stored via pointers/smart pointers.
- Existing project documentation (e.g., `docs/game_class_diagram.txt`) to confirm where virtual methods are declared and how derived types like `Player`, `Enemy`, `Boss`, `Platform`, `Projectile`, `FireProjectile`, `IceProjectile`, and `HPPotion` fit into the hierarchy.

**Use of GenAI (GPT) in Completing This Concept:**

- **Did you use GenAI?**

  - Yes. Generative AI (GPT) was used to help articulate how polymorphism is applied across the `Entity`, `Character`, and `Projectile` hierarchies and to structure the written report section.

- **Prompts Used (Examples):**

  - “Write a Report Section for Polymorphism, containing: 1.2 Polymorphism, 1.2.1 Application, 1.2.2 Concept, 1.2.3 Implementation & Output, 1.2.4 Troubleshooting Summary, for my platformer game.”
  - Follow-up prompts to ensure all relevant classes (`Entity`, `Character`, `Player`, `Enemy`, `Boss`, `Platform`, `Projectile`, `FireProjectile`, `IceProjectile`, `HPPotion`) were included.

- **Snapshots of Key GPT Responses:**
  - Screenshots of the relevant GPT conversation should be captured by the student/user and embedded here, for example:
    - `![GPT Response – Polymorphism Concept](images/gpt-polymorphism-concept.png)`
    - `![GPT Response – Polymorphism Troubleshooting](images/gpt-polymorphism-troubleshooting.png)`
  - (These image paths are placeholders; replace them with actual screenshot paths in your project.)

**Cited Resources / References:**

- Stroustrup, B. _The C++ Programming Language_ – chapters on polymorphism and virtual functions.
- C++ reference documentation (e.g., `https://en.cppreference.com/`) for details on virtual dispatch, `override`, and object slicing.
- Game programming resources discussing polymorphism-based entity systems for 2D games and platformers.
