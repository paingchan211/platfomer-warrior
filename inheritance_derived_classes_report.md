## 1.1 Inheritance and Derived Classes

### 1.1.1 Application

**Application Area:** Entity and character hierarchy for the platformer (player, enemies, boss).

In this platformer, many on-screen actors share common behavior:

- All visible actors (player, enemies, boss, possibly projectiles) have a sprite, a position, a velocity, and an “active” flag.
- Combat-capable actors (player, enemies, boss) additionally share health, damage, movement speed, and “on ground” state.

From the class diagram:

- `Entity` is an abstract base that owns the core render/movement data:
  - `sf::Sprite sprite`
  - `sf::Vector2f position`, `velocity`
  - `bool active`
  - Pure virtual `update(float dt)` and a virtual `getBounds() const`
- `Character` extends this idea with combat stats and movement data:
  - `hp`, `maxHp`, `damage`, `speed`, `onGround`
  - `takeDamage(int)`, `isAlive() const`, getters/setters, and `initializeCombatStats(...)`
- Concrete gameplay actors such as `Player`, `Enemy`, and `Boss` build on these foundations, adding:
  - Player-specific input handling, jump and dash logic, and animation state.
  - Enemy and boss-specific AI and attack patterns.

**Desired Operation (Detailed):**

- The game world (`GameWorld`) should be able to:
  - Create and manage a `Player`, multiple `Enemy` instances, and a single `Boss`.
  - Update them each frame via a common interface (`update(float dt)`), without duplicating position or rendering code.
  - Query their bounding boxes (`getBounds()`) for collision detection.
- Combat and game logic should work in a unified way for all characters:
  - Damage, HP checks, and “is alive” logic should be shared, so changing the damage system only requires modifying one place.
  - Movement-related concepts (speed, being on the ground) should be consistent across all characters.
- Rendering should consistently use the `Entity`’s `sprite` and `position`, so that:
  - The camera and world systems don’t care whether they are dealing with the player or an enemy; they just use the base interface.

Inheritance and derived classes are used here specifically to avoid duplicating common behavior and to allow polymorphic updates and collision checks across a heterogeneous set of entities.

### 1.1.2 Concept

**How Inheritance Matches the Desired Operation:**

- **Shared Core Behavior (Entity Base Class):**
  - `Entity` encapsulates the minimal data and operations that every in-world actor needs: position, velocity, active flag, sprite, and bounds.
  - By making `update(float dt)` pure virtual, `Entity` guarantees that every derived class will implement its own update logic, while still allowing the game loop to treat all entities uniformly.

- **Shared Combat and Movement Logic (Character Base Class):**
  - `Character` extends `Entity` with health, damage, speed, and ground state.
  - Functions like `takeDamage`, `isAlive`, and `initializeCombatStats` centralize combat rules; the player, enemies, and boss all follow the same basic HP/damage semantics.
  - This ensures that balance changes (like adjusting starting HP or damage) can be made in one place and affect all derived characters consistently.

- **Specialized Derived Classes (Player, Enemy, Boss):**
  - `Player` inherits from `Character` and adds:
    - Input handling (movement, jumping, attacking).
    - Player-specific animation state (current frame, timers, facing direction).
    - Special actions (dash, special attacks).
  - `Enemy` and `Boss` inherit from `Character` and add:
    - AI behavior (chasing the player, patrol logic, attack patterns).
    - Boss-specific mechanics such as rage mode or meteor attacks.
  - Because they share the same base interfaces, systems like physics, collision, and combat can work with `Character&` or `Entity&`, regardless of the concrete derived type.

- **Polymorphic Updates and Collisions:**
  - The game loop and physics manager can keep collections of pointers/references to `Entity` or `Character` and call `update(dt)` or `getBounds()` on each without needing `if`/`switch` on type.
  - New entity types (e.g., NPCs or traps) can be added later by deriving from `Entity`/`Character` and slotting into the same update/collision pipeline.

Overall, inheritance and derived classes map directly to the design goal of having many different actors in the world that:

- Share a consistent core representation (position, sprite, HP, damage).
- Behave differently in their own `update` implementations.
- Can still be operated on through a single, unified base interface.

**Alternative Structures and Why They Are Less Preferred Here:**

1. **No Inheritance, Only Separate Classes (Copy/Paste Fields):**
   - Pros:
     - Straightforward to start: `Player`, `Enemy`, and `Boss` each have their own position, sprite, hp, damage, etc.
   - Cons:
     - Massive code duplication: every class repeats the same fields and logic for movement, damage, and rendering.
     - Any change to shared behavior (e.g., new HP rules) must be manually kept in sync across all classes.
   - Conclusion:
     - Becomes very error-prone and hard to maintain as the number of entity types grows.

2. **Composition-Only Approach (No Polymorphic Base):**
   - Pros:
     - Use components such as `Transform`, `Health`, `SpriteRenderer` and attach them to plain structs.
     - Very flexible; often used in full ECS (Entity Component System) architectures.
   - Cons:
     - Requires more infrastructure and boilerplate (systems that know how to update each component type).
     - For a student-scale platformer, a full ECS-style architecture is likely overkill compared to a simple inheritance tree.
   - Conclusion:
     - Composition is powerful long-term, but inheritance offers a simpler and more approachable way to share behavior and enable polymorphism in this project.

3. **Type-Erased or Variant-Based Entity Representation:**
   - Pros:
     - Could store different entity types in a `std::variant` or type-erased wrapper.
     - Avoids virtual functions in some designs.
   - Cons:
     - Adds complexity to every place where behavior differs; typically requires manual visitors or `std::visit` with switch-like logic.
     - Harder to explain and maintain than straightforward inheritance for this kind of platformer.
   - Conclusion:
     - Not as clear or educational as using classical inheritance and virtual methods.

For this platformer, classical inheritance (`Entity` → `Character` → `Player`/`Enemy`/`Boss`) is preferred because it:

- Minimizes duplication of core logic and data.
- Provides a clean polymorphic interface for updates and collisions.
- Stays readable and appropriate for a teaching/assignment context.

### 1.1.3 Implementation & Output

This section lists the key files and code fragments that implement the inheritance hierarchy used for entities and characters in the platformer.

**File 1: `Entity.h` – Abstract Base Class for All In-World Entities**

- **Purpose:** Provides the minimal data and interface that every in-world actor (player, enemies, boss, etc.) shares.
- **Key Code Fragment:**

```cpp
class Entity
{
protected:
    sf::Sprite sprite;
    sf::Vector2f position;
    sf::Vector2f velocity;
    bool active;

public:
    Entity();
    virtual ~Entity() = default;

    virtual void update(float dt) = 0;
    virtual sf::FloatRect getBounds() const;

    sf::Vector2f getPosition() const;
    sf::Vector2f getVelocity() const;
    bool isActive() const;
    const sf::Sprite &getSprite() const;

    void setPosition(sf::Vector2f pos);
    void setVelocity(sf::Vector2f vel);
    void setActive(bool isActive);
};
```

**File 2: `Character.h` – Base Class for Combat-Capable Entities**

- **Purpose:** Extends `Entity` with health, damage, movement speed, and “on ground” state used by all combat characters.
- **Key Code Fragment:**

```cpp
class Character : public Entity
{
protected:
    int hp;
    int maxHp;
    int damage;
    bool onGround;
    float speed;

public:
    Character();
    virtual ~Character() = default;

    virtual void takeDamage(int dmg);
    virtual bool isAlive() const;

    int getHp() const;
    int getMaxHp() const;
    int getDamage() const;
    bool getOnGround() const;
    float getSpeed() const;

    void setHp(int health);
    void setMaxHp(int maxHealth);
    void setDamage(int dmg);
    void setOnGround(bool ground);
    void setSpeed(float spd);

protected:
    void initializeCombatStats(int initialHp, int initialDamage);
};
```

**File 3: `Player.h` – Player Character Derived from `Character`**

- **Purpose:** Implements the controllable player character, inheriting all combat and movement data from `Character` and adding input, animation, jumping, dashing, and progression systems.
- **Key Code Fragment (class header and important overrides):**

```cpp
class Player : public Character
{
public:
    struct LevelUpInfo
    {
        bool leveledUp;
        int newLevel;
        int hpGained;
        int damageGained;

        LevelUpInfo()
            : leveledUp(false), newLevel(0), hpGained(0), damageGained(0) {}
    };

    Player(Animation *run, Animation *idle, Animation *atk1, Animation *atk2,
           float startX, float startY, float charHeight, float grav, float jump);

    void update(float dt) override;
    sf::FloatRect getBounds() const override;

    // Additional player-specific methods:
    void handleInput(float dt, const KeyBindingManager *keyManager = nullptr, bool sprintDisabled = false);
    void updatePhysics(float dt, float groundLevel, Platform **platforms, int numPlatforms);
    // ... (leveling, inventory, attack, and special-attack helpers)
};
```

**File 4: `Enemy.h` – Enemy Character Derived from `Character`**

- **Purpose:** Represents regular enemies that share `Character` stats and add AI, patrol, chase, attack, and status-effect behavior.
- **Key Code Fragment (class header and override):**

```cpp
enum class EnemyState
{
    PATROLLING,
    CHASING,
    ATTACKING
};

class Enemy : public Character
{
public:
    Enemy(float x, float y, float leftBound, float rightBound,
          const EnemyAnimation &anim);

    void update(float dt) override;              // base override (no-op)
    void update(float dt, float gravity, const Player &player);
    sf::FloatRect getBounds() const override;
    sf::FloatRect getCollisionBounds() const;
    sf::FloatRect getAttackBounds() const;

    // Additional enemy-specific methods and state accessors...
};
```

**File 5: `Boss.h` – Boss Character Derived from `Enemy`**

- **Purpose:** Extends `Enemy` with boss-specific state machine (spawn, idle, chasing, attacking), regeneration, and unique attack behavior.
- **Key Code Fragment (class header and important methods):**

```cpp
enum class BossState
{
    INACTIVE,
    SPAWNING,
    IDLE,
    PATROLLING,
    CHASING,
    ATTACKING
};

class Boss : public Enemy
{
public:
    explicit Boss(const BossAnimation &bossAnim);

    void spawn(sf::Vector2f spawnPos);
    void update(float dt, float gravity, const Player &player, bool isEnraged = false);
    sf::FloatRect getBounds() const override;
    sf::FloatRect getCollisionBounds() const;

    bool isAlive() const override;

    BossState getBossState() const;
    bool getIsAttacking() const;
    int getAttackFrame() const;
    float getAttackRange() const;
    const BossAnimation &getAnimation() const;

    void setBossState(BossState newState);
    sf::FloatRect computeAttackBounds(bool playerOnRight) const;
};
```

Together, these files show how the inheritance chain:

- Starts from a minimal `Entity` base,
- Adds shared combat and movement logic in `Character`,
- And then specializes behavior for `Player`, `Enemy`, and `Boss` while still using the same virtual interfaces (`update`, `getBounds`, `isAlive`, etc.).

### 1.1.4 Troubleshooting Summary

**Issues Encountered During Implementation:**

- **Issue 1: Choosing the Right Base Class Boundaries**
  - **Problem:** It was initially unclear which properties belonged in `Entity` versus which should be in `Character`. Some combat-related fields (like HP and damage) were temporarily duplicated in derived classes.
  - **Root Cause:** The responsibility split between “all entities” and “only combat-capable entities” was not fully defined at the start.
  - **Solution:** Refactored the hierarchy to:
    - Keep purely spatial/rendering data (`sprite`, `position`, `velocity`, `active`, `getBounds()`) in `Entity`.
    - Move HP, damage, speed, and ground state into `Character`.
    - Ensure `Player`, `Enemy`, and `Boss` inherit from `Character` so combat logic is centralized.

- **Issue 2: Virtual Functions and Destructors**
  - **Problem:** There was concern about deleting derived objects (e.g., via a pointer to `Entity` or `Character`) and whether destructors would run correctly.
  - **Root Cause:** Without a virtual destructor in the base class, deleting derived objects through a base pointer can cause undefined behavior and resource leaks.
  - **Solution:** Confirmed that `Entity` and `Character` have virtual destructors (`virtual ~Entity()`, `virtual ~Character()`), ensuring that when a derived object is destroyed through a base pointer, the correct destructor chain is executed.

- **Issue 3: Polymorphic Updates and Object Slicing**
  - **Problem:** Early designs risked storing `Entity` or `Character` objects by value in containers, which can slice off derived parts and break polymorphic behavior.
  - **Root Cause:** Copying derived objects into containers of base type (instead of storing pointers/references) causes “object slicing,” removing derived data and vtable information.
  - **Solution:** Ensured that polymorphic entities (player, enemies, boss) are managed via pointers or smart pointers (`std::unique_ptr<Player>`, arrays of `std::unique_ptr<Enemy>`, etc.), and not by value in base-type containers. This preserves correct virtual dispatch of `update(dt)` and other overridden methods.

- **Issue 4: Aligning Physics/Collision Code with the Hierarchy**
  - **Problem:** Physics and collision code initially handled player and enemies separately, which duplicated logic and made maintenance harder.
  - **Root Cause:** The code was not yet fully refactored to rely on the shared `Entity`/`Character` interface (e.g., `getBounds()`, `getPosition()`).
  - **Solution:** Gradually migrated physics and collision routines to work with base-class interfaces where practical:
    - Use `Entity`’s `getBounds()` for collision checks.
    - Use shared position/velocity accessors in movement code.
    - Reserve specialized behavior (AI, input) for the derived classes.

**Resources / References Used:**

- C++ object-oriented programming references and tutorials, focusing on:
  - Inheritance, virtual functions, and abstract base classes.
  - Proper use of virtual destructors to avoid leaks when deleting through base pointers.
- The game’s own class diagram (`docs/game_class_diagram.txt`) to verify the final hierarchy and relationships between `Entity`, `Character`, `Player`, `Enemy`, and `Boss`.
- General game architecture articles explaining common entity hierarchies in 2D platformers.

**Use of GenAI (GPT) in Completing This Concept:**

- **Did you use GenAI?**
  - Yes. Generative AI (GPT) was used to help structure and articulate the inheritance design in the context of the existing platformer class diagram.

- **Prompts Used (Examples):**
  - “Write a Report Section for Inheritance and Derived Classes, containing 1.1 Inheritance and Derived Classes, 1.1.1 Application, 1.1.2 Concept, 1.1.3 Implementation & Output, 1.1.4 Troubleshooting Summary.”
  - Follow-up prompts to tie the explanation specifically to the `Entity` → `Character` → `Player`/`Enemy`/`Boss` hierarchy.

- **Snapshots of Key GPT Responses:**
  - Screenshots of the relevant GPT conversation should be captured by the student/user and embedded here, for example:
    - `![GPT Response – Inheritance Concept](images/gpt-inheritance-concept.png)`
    - `![GPT Response – Inheritance Troubleshooting](images/gpt-inheritance-troubleshooting.png)`
  - (These image paths are placeholders; replace them with actual screenshot paths in your project.)

**Cited Resources / References:**

- Stroustrup, B. _The C++ Programming Language_ – sections on classes, inheritance, and virtual functions.
- C++ reference documentation (e.g., `https://en.cppreference.com/`) for details on virtual destructors and object lifetime.
- Game development resources discussing entity hierarchies and OOP design in 2D games (articles and tutorials on platformer entity design).
