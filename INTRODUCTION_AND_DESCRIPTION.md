# Introduction and Description

## **1.1 Project Overview**

This report documents the design, implementation, and technical architecture of a comprehensive 2D action platformer game developed in C++ using the SFML (Simple and Fast Multimedia Library) framework. The project demonstrates advanced programming concepts including object-oriented design, custom data structure implementations, design pattern applications, and real-time game system development with **AI Storyteller** and **procedural storytelling** mechanics.

The game combines traditional platformer mechanics with modern action-RPG elements, featuring a progression system, skill trees, dynamic combat, and challenging boss encounters. At its core, a **GameMaster** system orchestrates procedural events and difficulty modifiers, creating unique gameplay experiences through dynamic environmental hazards, adaptive combat scenarios, and emergent narrative moments. The project serves as both an engaging gameplay experience and a technical showcase of computer science principles applied to interactive entertainment software.

**Development Stack:**

- **Programming Language:** C++17
- **Graphics Framework:** SFML 2.5.1
- **Development Environment:** Visual Studio Code
- **Compiler:** MinGW g++ (GCC 7.3.0)
- **Build System:** Custom batch/PowerShell scripts
- **Version Control:** Git

**Project Scale:**

- **Source Files:** 50+ C++ implementation and header files
- **Lines of Code:** Approximately 8,000+ lines
- **Custom Data Structures:** 6 implementations (Hash Table, Linked Lists, Stack, Queue, Tree)
- **Design Patterns:** 5+ patterns (Singleton, Factory, State, Visitor, Object Pooling)
- **Game Assets:** 20+ texture files, multiple audio assets
- **Procedural Systems:** GameMaster AI for dynamic storytelling and event generation

---

## **1.2 Game Genre and Core Mechanics**

**Genre Classification:** Side-scrolling Action Platformer with RPG Progression Elements and Procedural Storytelling

The game belongs to the action platformer genre, emphasizing fast-paced combat, precise jumping mechanics, and character advancement. Players navigate a horizontally expansive 2D world (6000 pixels wide by 800 pixels tall), engaging in real-time combat with diverse enemy types while collecting experience, upgrading abilities, and ultimately confronting a powerful boss enemy. The **GameMaster** system acts as an **AI Storyteller**, dynamically adjusting gameplay intensity through **procedural storytelling** techniques—spawning environmental hazards, modifying enemy behavior, and creating dramatic moments based on player performance and health status.

**Core Gameplay Pillars:**

1. **Platforming Mechanics**

   - Fluid horizontal movement with sprint capability
   - Gravity-based physics simulation (800 pixels/second²)
   - Double-jump system for enhanced aerial mobility
   - Platform traversal across 10 unique platforms at varying elevations
   - Precise collision detection and response

2. **Combat Systems with Procedural Events**

   - Real-time melee combat with multiple attack animations
   - Ranged combat through fire and ice projectile systems
   - Special dash attack with multi-enemy hit capability
   - Status effect mechanics (burn damage-over-time, freeze/slow effects)
   - Dynamic difficulty modifiers orchestrated by the **GameMaster AI** (rage modes, environmental hazards)
   - **Procedural storytelling** through combat event variations and adaptive enemy behavior

3. **Character Progression**

   - Experience-based leveling system with stat increases
   - Skill tree with branching upgrade paths
   - Inventory system with healing potion collection
   - Save/load functionality for persistent progression

4. **Environmental Interaction and Procedural Events**
   - Moving platform mechanics
   - **Meteor hazard system** with **procedural storytelling** - dynamic spawn rates controlled by **GameMaster AI**
   - **Sandstorm event** that reduces visibility, softens music volume, and disables sprinting to spike tension
   - Ground and platform collision physics
   - Camera system following player through scrolling world
   - **AI-driven environmental storytelling** through hazard intensity scaling
   - Tunable gameplay constants are centralized in `Constants.h` (e.g., meteor frequency, late-enemy scaling, boss entrance offsets) for quick balancing

---

## **1.3 Narrative Context and Game Flow**

**Setting:** The game takes place in a fantasy realm under siege by hostile forces. The player controls a skilled warrior tasked with clearing a corrupted region by defeating waves of enemies and ultimately confronting the area's boss guardian. The **GameMaster** serves as an **AI Storyteller**, dynamically crafting the pacing and intensity of the player's journey through **procedural storytelling** mechanics—adjusting environmental threats, triggering dramatic events, and creating emergent narrative moments based on the evolving combat situation.

**Gameplay Progression Flow with Procedural Storytelling:**

**Phase 1 - Early Game (Minutes 0-5):**

- Player spawns at the western edge of the game world
- Tutorial-like introduction through first enemy encounters
- Learning movement controls, jumping, and basic combat
- First level-up occurs, introducing progression mechanics
- Unlock initial skill tree abilities (Special Attack or Fire Projectile)
- **GameMaster** monitors player performance and adjusts difficulty accordingly

**Phase 2 - Mid Game (Minutes 5-10):**

- Encounter multiple enemies simultaneously, requiring tactical combat
- Experimentation with unlocked abilities (projectiles, special attacks)
- Collection of HP potion drops from defeated enemies
- Platform navigation challenges increase in complexity
- Level 2-4 progression unlocks skill tree branches
- Approximately 5-8 enemy defeats across the map
- **Procedural events** begin: occasional meteor hazards spawn based on **GameMaster** difficulty assessment

**Phase 3 - Boss Preparation (Minutes 10-12):**

- Defeat of final regular enemy triggers boss spawn sequence
- "All enemies defeated! Boss approaching..." alert displays
- Boss spawn animation at the eastern edge of the world
- Player has opportunity to prepare (heal, position strategically)
- Dramatic music/visual changes signal boss encounter
- **GameMaster AI** transitions to boss-phase storytelling mode

**Phase 4 - Boss Battle with AI Storyteller Events (Minutes 12-18):**

- Initial engagement phase: learning boss attack patterns
- **Procedural storytelling** through health-threshold events:
  - Boss rage mode triggers at 50% HP (**GameMaster**-orchestrated event)
  - Player rage mode activates when critically injured (<30% HP) with nearby enemies
  - **Meteor Fury Mode**: **GameMaster AI** dramatically increases meteor spawn frequency (1-2 second intervals) when player is critically wounded during boss fight
- **AI Storyteller** creates tension through layered procedural events:
  - Environmental hazard escalation (meteor showers)
  - Boss behavior modifications (increased aggression, speed, damage)
  - Visual/audio cues for dramatic intensity shifts
- Final damage phase requires full utilization of player abilities
- Boss defeat triggers slow-motion cinematic effect (3 seconds at 30% speed)—a **procedural storytelling** reward moment orchestrated by **GameMaster**
- Victory screen displays final statistics and achievements

**Victory Conditions:**

- Boss HP reduced to zero
- Player survives the encounter

**Defeat Conditions:**

- Player HP reduced to zero
- Game over screen with retry option

---

## **1.4 Technical Architecture Overview**

The game employs a **modular, component-based architecture** with clear separation of concerns across multiple subsystems. Central to this design is the **GameMaster** component, which functions as an **AI Storyteller** to orchestrate **procedural storytelling** events throughout gameplay. This design philosophy promotes code maintainability, testability, and extensibility.

**System Architecture Hierarchy:**

```
┌─────────────────────────────────┐
│     Game (Singleton)            │  ← Entry point and lifecycle manager
│  - Single global instance       │
│  - Runs main game loop          │
└────────────┬────────────────────┘
             │
             ▼
┌─────────────────────────────────┐
│         Session                 │  ← Manages single gameplay session
│  - Window management            │
│  - Core system coordination     │
│  - Game state stack             │
│  - Integrates GameMaster AI     │
└────┬────┬────┬────┬────┬────┬──┘
     │    │    │    │    │    │
     ▼    ▼    ▼    ▼    ▼    ▼
   ┌──┐ ┌──┐ ┌──┐ ┌──┐ ┌──┐ ┌──┐
   │GW│ │UI│ │CS│ │PM│ │CC│ │GM│  ← Specialized subsystems
   └──┘ └──┘ └──┘ └──┘ └──┘ └──┘

GW = GameWorld    UI = UISystem      CS = CombatSystem
PM = PhysicsManager    CC = CameraController    GM = GameMaster (AI Storyteller)
```

**Core Subsystem Responsibilities:**

1. **Game (Singleton Pattern)**

   - Provides single global access point to game instance
   - Manages high-level application lifecycle (start, stop, exit)
   - Ensures resource manager initialization
   - Prevents multiple game instance creation

2. **Session**

   - Orchestrates single gameplay session from menu to game over
   - Contains SFML render window (1200×800 resolution)
   - Maintains game state stack for menu/gameplay transitions
   - Coordinates updates across all subsystems
   - Implements main game loop (event processing, update, render)
   - Integrates **GameMaster AI Storyteller** for procedural event coordination

3. **GameWorld**

   - Entity factory and management
   - World configuration (floor tiles, platforms, spawn positions)
   - Player, enemy, and boss instantiation
   - Level geometry setup (10 platforms, 13 floor tiles)
   - Entity lifecycle management

4. **Player**

   - Character controller with complex state machine
   - Animation system (idle, run, attack1, attack2)
   - Physics integration (gravity, jump, collision)
   - Progression tracking (level, XP, stats)
   - Skill tree management
   - Inventory system

5. **CombatSystem**

   - Melee attack processing and hit detection
   - Projectile lifecycle management (fire/ice projectiles)
   - Object pooling for projectiles, potions, meteors
   - Status effect application (burn, freeze, slow)
   - Combat event callbacks (damage dealt, enemy defeated)
   - Ammo and cooldown management
   - Interfaces with **GameMaster** for combat-driven procedural events

6. **UISystem**

   - HUD rendering (health bars, XP bar, ammo indicators)
   - Menu rendering (main menu, pause menu, skill tree, inventory)
   - Floating damage text display
   - Status effect visualization
   - Screen overlays (rage mode, low HP warning)
   - Toast notification system
   - Visual feedback for **GameMaster**-triggered events

7. **PhysicsManager**

   - Collision detection (AABB - Axis-Aligned Bounding Boxes)
   - Gravity application
   - Platform collision response
   - Ground collision handling
   - Attack range calculation

8. **CameraController**

   - Smooth camera following of player position
   - World boundary clamping (prevents camera showing outside world)
   - Dynamic view positioning (keeps player centered horizontally)

9. **ResourceManager**

   - Asset loading and caching (textures, fonts, sounds)
   - Memory management for shared resources
   - Texture atlas management
   - Font loading for UI text

10. **InputManager**

    - Keyboard input polling
    - Input action mapping
    - Debouncing and cooldown management
    - State-aware input handling (different actions in different game states)

11. **KeyBindingManager**

    - Customizable key bindings
    - Configuration file persistence (keybindings.cfg)
    - Default binding restoration
    - Input conflict detection

12. **GameMaster (AI Storyteller System)**

    - **Procedural storytelling** orchestration and event generation
    - **AI-driven difficulty modifiers** based on player performance
    - Dynamic spawn rate adjustments for environmental hazards
    - Rage mode activation logic (player and boss)
    - **Meteor Fury Mode** event triggering when player critically injured
    - Game event coordination and pacing control
    - Creates emergent narrative moments through procedural event sequencing
    - Monitors player health, enemy count, and combat status to trigger appropriate story beats
    - Acts as **AI Storyteller** by interpreting gameplay state and responding with dramatic events

13. **SaveGameManager**
    - Game state serialization to text files
    - Save slot management (multiple save files supported)
    - Player stat persistence (HP, damage, level, XP)
    - Skill tree progression saving
    - Inventory state saving

---

## **1.5 Key Technical Features**

**Advanced Data Structure Implementations:**

The game showcases custom implementations of fundamental data structures, demonstrating understanding of algorithmic complexity and appropriate data structure selection:

| Data Structure         | Use Case                                                             | Operations               | Time Complexity            |
| ---------------------- | -------------------------------------------------------------------- | ------------------------ | -------------------------- |
| **Hash Table**         | Resource management (texture/sound lookup by string key)             | Insert, Lookup           | O(1) average               |
| **Singly Linked List** | Combat log messages (sequential insertion, procedural event logging) | Insert, Traverse         | O(1) insert, O(n) traverse |
| **Doubly Linked List** | Projectile/potion/meteor pools (bidirectional traversal)             | Insert, Remove, Traverse | O(1) at ends, O(n) search  |
| **Stack (LIFO)**       | Game state management (push/pop menus)                               | Push, Pop, Peek          | O(1)                       |
| **Queue (FIFO)**       | Floating damage text (temporal ordering of procedural events)        | Enqueue, Dequeue         | O(1)                       |
| **N-ary Tree**         | Skill tree (hierarchical dependencies)                               | Insert, Search, Traverse | O(log n) balanced          |

**Design Pattern Applications:**

1. **Singleton Pattern** - Game instance (prevents multiple game instances)
2. **Factory Pattern** - Entity creation through GameWorld
3. **State Pattern** - Game state stack for menu/gameplay transitions
4. **Visitor Pattern** - Rendering and debug visualization
5. **Object Pooling Pattern** - Projectile, potion, and meteor reuse (enables efficient procedural event spawning)

**Physics and Collision Systems:**

- **Gravity Simulation:** Constant acceleration (800 pixels/second²)
- **Jump Mechanics:** Instant upward velocity (-600 pixels/second) with double-jump support
- **Collision Detection:** AABB (Axis-Aligned Bounding Box) with margin-based precision
- **Platform Collision:** One-way platforms (can jump through from below)
- **Attack Range Detection:** Directional hitbox extending from player facing direction

**Animation System:**

- **Sprite Sheet Management:** Frame-based animation with configurable durations
- **State-Driven Animation:** Animations tied to character states (idle, run, attack)
- **Directional Sprites:** Horizontal flipping based on facing direction
- **Frame Interpolation:** Smooth transitions between animation frames

**Status Effect System:**

- **Burn (Fire):** Damage-over-time effect, ticks every 1 second
- **Ice Stacks:** Accumulating slow effect; 2+ stacks freeze target completely
- **Freeze:** 0% movement speed for duration
- **Slow:** 50% movement speed reduction

**Dynamic Difficulty Systems (GameMaster AI Storyteller):**

- **Player Rage Mode:** **Procedural event** activated at <30% HP with 2+ nearby enemies, grants 2× damage for 8 seconds—**GameMaster** interprets desperation and grants power boost
- **Boss Rage Mode:** **AI-triggered event** at <50% boss HP, increases damage, speed, and attack frequency by 1.5-2×—**procedural storytelling** through boss behavior escalation
- **Meteor Fury:** **GameMaster**-orchestrated environmental storytelling—hazard spawn rate increases dramatically (1-2 second intervals) when player critically injured during boss fight, creating tension through **procedural event generation**

---

## **1.6 GameMaster AI Storyteller System (Procedural Storytelling Architecture)**

**Concept:** The **GameMaster** component functions as an **AI Storyteller**, monitoring gameplay state in real-time and orchestrating **procedural storytelling** through dynamic event generation. Rather than following a fixed narrative script, the game creates emergent story moments by analyzing player health, combat performance, enemy status, and game progression—then responding with appropriate dramatic events.

**Procedural Storytelling Mechanics:**

### **Event-Driven Narrative Generation**

The **GameMaster AI** implements several **procedural storytelling** systems:

**1. Health-Based Story Triggers**

- Monitors player HP percentage continuously
- At **<40% HP**: Triggers low HP warning visual effects (red screen flash)—subtle story beat signaling danger
- At **<30% HP + 2+ enemies nearby**: **GameMaster** spawns **Player Rage Mode** event—narrative moment of "desperation power"
- Creates tension arc: player moves from confidence → danger → desperation → empowerment

**2. Boss Phase Storytelling**

- **GameMaster** tracks boss HP thresholds to create multi-act boss battle narrative
- **Act 1 (100-50% HP)**: Learning phase, boss uses standard attacks
- **Act 2 (<50% HP)**: **Procedural event** "Boss Enrage"—**AI Storyteller** escalates difficulty, changes boss behavior, adds visual effects
- **Act 3 (Critical HP)**: Environmental storytelling intensifies through **Meteor Fury Mode**
- Each phase creates distinct story beat: introduction → escalation → climax

**3. Environmental Hazard Storytelling**

- **Meteor System** as **procedural narrative device**:
  - **Normal Mode (10-20 sec intervals)**: Background environmental danger
  - **Meteor Fury Mode (1-2 sec intervals)**: **GameMaster**-triggered dramatic escalation when player is critically wounded during boss fight
  - **AI Storyteller** uses meteor frequency to communicate urgency and desperation
  - Creates procedural "everything is falling apart" moment without scripted cutscenes

**4. Combat Event Logging as Story Chronicle**

- Combat log (Singly Linked List) serves as **procedural narrative chronicle**
- **GameMaster** generates contextual messages for events:
  - "Player dealt 45 damage to Enemy with Fire Projectile!"
  - "Boss ENRAGED! Damage and speed increased!"
  - "Player entered RAGE MODE! Damage doubled!"
  - "Meteor Fury activated! Survive the onslaught!"
- Creates emergent story through event sequencing

**5. Victory/Defeat Story Moments**

- Boss defeat triggers **procedural cinematic event**:
  - **GameMaster** activates slow-motion effect (30% time scale for 3 seconds)
  - Camera focuses on boss defeat animation
  - **AI Storyteller** creates dramatic payoff moment through timing and visual effects
- Even technical game over becomes story beat: "Your journey ends here..."

### **AI Storyteller Decision Logic**

```
GameMaster Procedural Event Loop:

Every frame:
  1. Assess player state (HP, position, rage status)
  2. Assess combat state (enemy count, boss HP, active projectiles)
  3. Assess environmental state (meteor count, active hazards)

  If player HP < 30% AND nearby_enemies >= 2 AND rage_on_cooldown == false:
      → Trigger "Rage Mode" procedural event
      → Log dramatic message: "RAGE MODE ACTIVATED!"
      → Apply 2× damage multiplier for 8 seconds
      → Create red screen pulse effect

  If boss HP < 50% AND boss_rage_triggered == false:
      → Trigger "Boss Enrage" procedural event
      → Log dramatic message: "BOSS ENRAGED!"
      → Multiply boss stats by 1.5-2×
      → Change boss visual (red tint)

  If player HP < 20% AND boss_alive AND meteor_fury_inactive:
      → Trigger "Meteor Fury" procedural event
      → Change meteor spawn interval from 10-20s to 1-2s
      → Log warning: "Meteor Fury activated!"
      → Create visual intensity through hazard density

  If boss HP <= 0:
      → Trigger "Victory Slowmotion" procedural cinematic
      → Apply time_scale = 0.3 for 3 seconds
      → Play victory music
      → Display victory screen after cinematic
```

### **Procedural Storytelling Benefits**

**1. Replayability Through Emergent Narrative**

- No two playthroughs feel identical
- Story beats occur at different times based on player skill
- Skilled players may never trigger rage mode; struggling players get help
- **AI Storyteller** adapts narrative pacing to individual player

**2. Player Agency in Story Creation**

- Player's combat decisions affect when/how story events trigger
- Aggressive playstyle → quick boss phases
- Defensive playstyle → more environmental hazard events
- Player "writes" their own story through gameplay choices

**3. Dynamic Difficulty as Storytelling Tool**

- **GameMaster** uses difficulty modifiers to tell "underdog comeback" or "overwhelming odds" stories
- Procedural events create dramatic tension without taking control from player
- Story emerges from systems interaction rather than scripted sequences

**4. Technical Storytelling Through Systems**

- No dialogue or cutscenes needed—story told through:
  - Visual effects (screen overlays, slow motion)
  - Audio cues (rage activation sounds)
  - Gameplay modifiers (damage multipliers, speed changes)
  - Environmental changes (meteor frequency)
  - UI messages (combat log, alerts)

---

## **1.7 Visual and Audio Design**

**Art Style:**

- Pixel art/sprite-based character designs
- Fantasy medieval aesthetic
- Distinct visual design for player, enemies, and boss
- Particle-like effects for projectiles and status indicators
- Visual feedback for **GameMaster**-triggered procedural events (screen overlays, color tints)

**Character Sprites:**

- **Player:** Warrior character (192 pixels tall) with sword-based combat animations
- **Enemies:** Lancer enemies (320 pixels tall) with spear attack animations
- **Boss:** Scaled 2.5× (800 pixels tall) for imposing presence
- **Animation Frames:** 4-8 frames per animation for smooth motion

**Environmental Art:**

- Floor tiles (490×196 pixels each, 13 tiles spanning 6000 pixel world width)
- Platforms (various sizes: 100-150 pixel widths)
- Background elements (single-color sky for clarity)

**Visual Effects for Procedural Storytelling:**

- Floating damage numbers (color-coded by damage type)
- Screen overlays (red tint for rage modes, flash for low HP)—**GameMaster**-triggered visual story beats
- Status effect icons above character heads
- Health bars (player, enemies, boss)
- Projectile trails and impact effects
- Slow-motion cinematic on boss defeat—**procedural storytelling** reward moment
- **Meteor visual warning system** for **AI Storyteller** environmental events

**Audio Design:**

- Attack sound effects (melee strikes)
- Projectile launch sounds
- Hit/damage impact sounds
- Enemy/boss death sounds
- Potion collection sounds
- Rage mode activation audio cue—**procedural event** sound design
- Ambient background music (during gameplay)

---

## **1.8 Recommended Screenshot Guide**

To comprehensively document your game, capture the following screenshots (organized by category):

### **A. Core Gameplay (5 screenshots)**

**Screenshot 1: "Player Combat Showcase"**

- **When to capture:** During active combat with 2-3 enemies on screen
- **What to include:** Player mid-attack animation, enemies reacting, HUD visible showing health/XP/ammo
- **Location:** Center-left area of map (around X=1500) with platform in background
- **Purpose:** Demonstrates core combat loop and visual design
- **Tip:** Press the attack key (default: J/K) while enemies are within range; ensure damage numbers are visible

**Screenshot 2: "Platforming Mechanics"**

- **When to capture:** Player in mid-air between two platforms
- **What to include:** Character jumping animation, multiple platforms at different heights, double-jump indicator if possible
- **Location:** Mid-section with platforms at Y=400-450 (around X=2500)
- **Purpose:** Showcases level design and movement mechanics
- **Tip:** Use double-jump to get higher in the air for dramatic effect

**Screenshot 3: "Projectile Combat"**

- **When to capture:** Immediately after firing fire or ice projectile
- **What to include:** Projectile sprite mid-flight, trajectory toward enemy, ammo count visible in HUD
- **Location:** Open area with enemy at medium range (around X=1800)
- **Purpose:** Demonstrates ranged combat system
- **Tip:** Press projectile key (default: I) with enemy in front; take screenshot as projectile travels

**Screenshot 4: "Status Effects in Action"**

- **When to capture:** Enemy affected by burn or freeze status
- **What to include:** Enemy with visible status icon, status effect visual (blue tint for frozen, flames for burning), enemy health bar
- **Location:** Any combat area
- **Purpose:** Shows combat depth and elemental systems
- **Tip:** Hit enemy with 2 ice projectiles for freeze effect (blue tint), or 1 fire projectile for burn (see DoT numbers)

**Screenshot 5: "Environmental Hazards (Procedural Events)"**

- **When to capture:** During meteor shower (multiple meteors falling)—**GameMaster AI Storyteller** event
- **What to include:** 3-5 meteors at different descent stages, player dodging, visual warning effects
- **Location:** Any outdoor area during boss fight or **Meteor Fury Mode** (procedural storytelling event)
- **Purpose:** Demonstrates **procedural storytelling** through environmental danger systems
- **Tip:** Wait for **Meteor Fury Mode** during boss fight (when your HP is low) for maximum meteor density—this is **GameMaster** creating dramatic tension

### **B. Boss Battle and Procedural Story Events (3 screenshots)**

**Screenshot 6: "Boss Introduction"**

- **When to capture:** Immediately after boss spawns
- **What to include:** Full boss sprite (2.5× scale), boss HP bar at top, player facing boss, "BOSS APPROACHING" alert if visible
- **Location:** Far right of map (boss spawn position around X=5500)
- **Purpose:** Shows climactic encounter design—**AI Storyteller** boss phase initialization
- **Tip:** Defeat all regular enemies, then wait for boss spawn animation to complete

**Screenshot 7: "Boss Rage Mode (Procedural Event)"**

- **When to capture:** After reducing boss HP below 50%—**GameMaster**-triggered procedural storytelling event
- **What to include:** Boss with red tint/glow, "BOSS ENRAGED" alert, boss HP bar showing <50%, intense screen effects
- **Location:** Boss arena during second phase
- **Purpose:** Demonstrates **procedural storytelling** through dynamic difficulty escalation—**AI Storyteller** creates Act 2
- **Tip:** Damage boss until HP bar is below halfway point; capture immediately when "BOSS ENRAGED" appears (this is **GameMaster** event)

**Screenshot 8: "Boss Defeat Slow Motion (Procedural Cinematic)"**

- **When to capture:** Immediately after boss HP reaches zero
- **What to include:** Boss death animation, slow-motion effect visual (if detectable), victory indicators
- **Location:** Boss arena
- **Purpose:** Shows **procedural storytelling** cinematic—**GameMaster** creates victory moment through time manipulation
- **Tip:** This happens automatically; be ready to screenshot right when boss HP depletes (game slows to 30% speed for 3 seconds—**AI Storyteller** victory payoff)

### **C. User Interface (6 screenshots)**

**Screenshot 9: "HUD Complete Overview"**

- **When to capture:** During normal gameplay with all HUD elements visible
- **What to include:** Health bar (top-left), XP bar (below health), inventory (HP potion count), ammo indicators (top-right), controls reminder (bottom)
- **Location:** Early-mid game with enemy visible for context
- **Purpose:** Documents UI/UX design comprehensively
- **Tip:** Ensure HP is not full (so bar shows current/max), have some XP progress, and have projectile ammo not at maximum

**Screenshot 10: "Skill Tree Interface"**

- **When to capture:** Press K key to open skill tree
- **What to include:** Complete skill tree layout, available skill points displayed, locked/unlocked skills visible, skill descriptions
- **Purpose:** Demonstrates progression and RPG systems
- **Tip:** Have at least 1 skill point available and at least 1 skill unlocked for visual variety

**Screenshot 11: "Player Stats Screen"**

- **When to capture:** Press P key to open stats
- **What to include:** Level display, HP/Max HP, damage stat, experience bar with numbers, equipped abilities list
- **Purpose:** Shows character advancement metrics
- **Tip:** Capture after gaining at least 1-2 levels for interesting stats

**Screenshot 12: "Combat Log (Procedural Event Chronicle)"**

- **When to capture:** Press L key to open combat log
- **What to include:** HP potion count, scrollable combat log messages (damage dealt, enemy defeats, level ups, **GameMaster events**)
- **Purpose:** Demonstrates inventory and **procedural storytelling** event logging system—**AI Storyteller** chronicle
- **Tip:** Capture after several combat encounters and **procedural events** so log has multiple entries including rage modes, boss events

**Screenshot 13: "Low HP Warning Effect"**

- **When to capture:** When player HP drops below 40%
- **What to include:** Red flashing screen overlay, low HP indicator, health bar in red zone
- **Location:** During combat with player taking damage
- **Purpose:** Shows dynamic visual feedback system—**GameMaster** subtle story beat signaling danger
- **Tip:** Let enemies hit you until HP < 40%; capture when red flash is visible

**Screenshot 14: "Rage Mode Overlay (Procedural Event)"**

- **When to capture:** When player rage mode activates (<30% HP with 2+ enemies nearby)—**GameMaster AI** procedural event
- **What to include:** Intense red screen pulse, "RAGE MODE" indicator, damage multiplier notification, aggressive combat
- **Location:** Combat area surrounded by multiple enemies
- **Purpose:** Illustrates **procedural storytelling** through dynamic gameplay modifier—**AI Storyteller** desperation power moment
- **Tip:** Deliberately take damage to ~25% HP with multiple enemies nearby; capture when red overlay pulses (this is **GameMaster** storytelling)

### **D. Menu Systems (4 screenshots)**

**Screenshot 15: "Main Menu"**

- **When to capture:** At game launch
- **What to include:** Game title, menu options (New Game, Load Game, Settings, Exit), background visuals
- **Purpose:** Documents game entry point
- **Tip:** This is the first screen; easy to capture

**Screenshot 16: "Pause Menu"**

- **When to capture:** Press ESC during gameplay
- **What to include:** Paused gameplay visible in background (slightly darkened), pause menu options overlay (Resume, Save, Load, Settings, Main Menu)
- **Purpose:** Shows game state management
- **Tip:** Pause during interesting gameplay moment so background looks engaging

**Screenshot 17: "Settings Menu"**

- **When to capture:** Navigate to Settings from main or pause menu
- **What to include:** Audio settings sliders, control customization options, gameplay settings
- **Purpose:** Documents configuration options
- **Tip:** Open settings submenu (Controls) to show key binding interface

**Screenshot 18: "Victory Screen (Procedural Story Conclusion)"**

- **When to capture:** After defeating boss
- **What to include:** Victory message, final player statistics (level, final HP, enemies defeated, time played), return to menu option
- **Purpose:** Documents win condition and **procedural storytelling** conclusion—**GameMaster** story resolution
- **Tip:** This appears automatically after boss defeat; shows game completion and **AI Storyteller** victory narrative

### **E. Technical/Optional (2 screenshots)**

**Screenshot 19: "World Overview"**

- **When to capture:** From an elevated position showing large world section
- **What to include:** Multiple platforms visible, floor tiles spanning distance, enemies at different positions
- **Location:** Mid-map elevated platform (around X=3000, Y=300)
- **Purpose:** Illustrates level design and world scale
- **Tip:** Stand on highest platform and let camera settle

**Screenshot 20: "Multi-Enemy Combat (Rage Mode Setup)"**

- **When to capture:** When surrounded by 3-4 enemies
- **What to include:** Player engaging multiple enemies simultaneously, tactical positioning—conditions for **GameMaster** rage mode trigger
- **Location:** Right side of map where multiple enemies spawn close together
- **Purpose:** Demonstrates AI and difficulty scaling—setup for **procedural storytelling** rage event
- **Tip:** Lure multiple enemies together by running between them without attacking; this creates conditions for **AI Storyteller** intervention

---

## **1.9 Screenshot Capture Tips**

**How to Take Screenshots:**

1. **Windows Built-in:** Press `Win + Shift + S` for Snipping Tool (select area)
2. **Print Screen:** Press `PrtScn` (copies entire screen to clipboard)
3. **Windows Game Bar:** Press `Win + G`, then camera icon
4. **SFML Screenshot:** Add code to save render window to file (if needed)

**Best Practices:**

- Ensure game window is in focus and at full 1200×800 resolution
- Capture during active gameplay moments (not idle screens)
- Include HUD elements for context (unless specifically documenting without HUD)
- Take multiple shots of each scenario and select the best one
- Use high-quality PNG format to preserve pixel art clarity
- Organize screenshots by naming convention: `01_PlayerCombat.png`, `02_Platforming.png`, etc.
- Capture **procedural events** in action (rage modes, meteor fury, boss enrage)—these show **GameMaster AI Storyteller** system

**Annotations for Report:**

- Add red arrows or circles to highlight specific features in report
- Include captions explaining what each screenshot demonstrates
- Reference screenshot numbers in report text: "(See Screenshot 3: Projectile Combat)"
- Label **procedural storytelling** elements: "GameMaster-triggered event," "AI Storyteller rage mode," etc.

---

## **1.10 Report Integration Structure**

When integrating screenshots into your report sections:

**Section 1 - Object-Oriented Programming:**

- Use Screenshot 1 (Player Combat) to show character inheritance
- Reference Screenshot 10 (Skill Tree) for polymorphism examples
- Mention **GameMaster** class as example of component design

**Section 2 - Composite Data Structures:**

- Screenshot 12 (Combat Log) demonstrates Singly Linked List usage for **procedural event** logging
- Screenshot 3 (Projectiles) shows Doubly Linked List for projectile management
- Screenshot 10 (Skill Tree) visualizes Tree data structure implementation
- Explain how data structures enable efficient **procedural storytelling** event handling

**Section 3 - Abstract Data Types:**

- Screenshot 16 (Pause Menu) illustrates Stack usage for game states
- Screenshot 12 (Floating damage) demonstrates Queue FIFO ordering for **procedural event** feedback

**Section 4 - Design Patterns:**

- Screenshot 15 (Main Menu) shows Singleton pattern entry point
- Screenshot 3 (Projectiles) demonstrates Object Pooling pattern (enables efficient **procedural event** spawning)
- Discuss **GameMaster** as example of Director/Orchestrator pattern for **AI Storyteller** functionality

**Procedural Storytelling Section:**

- Use Screenshots 7, 14 (Rage modes) to illustrate **GameMaster** event triggering
- Use Screenshot 5 (Meteor Fury) to show **AI Storyteller** environmental escalation
- Use Screenshot 12 (Combat Log) to demonstrate **procedural narrative** chronicle
- Use Screenshot 8 (Slow-motion victory) to show **procedural cinematic** storytelling

**General Documentation:**

- Use Screenshots 6-8 for boss system documentation and **procedural story phases**
- Use Screenshots 13-14 for dynamic difficulty systems (**GameMaster AI**)
- Use Screenshot 17-18 for menu and end-game systems

---

## **1.11 Conclusion**

This 2D action platformer demonstrates a sophisticated integration of traditional game development with modern **procedural storytelling** techniques. The **GameMaster AI Storyteller** system creates dynamic, player-responsive narratives without scripted cutscenes, instead generating story moments through:

- **Health-threshold event triggering** (rage modes at critical HP)
- **Combat-state responsive difficulty** (boss phases, meteor frequency)
- **Emergent narrative through system interaction** (player choices affect story pacing)
- **Technical storytelling** (visual effects, audio cues, gameplay modifiers tell the story)

The project showcases both technical computer science principles (data structures, design patterns, algorithms) and advanced game design concepts (**AI Storyteller**, **procedural event generation**, adaptive difficulty). By allowing gameplay state to drive narrative events, the game creates unique story experiences for each playthrough while maintaining consistent mechanical depth.

**Key Achievements:**

- **GameMaster AI** orchestrates **procedural storytelling** through 5+ event types
- Dynamic difficulty creates personalized narrative arcs (skilled vs. struggling players)
- Combat log serves as **procedural narrative chronicle** of player's unique journey
- Technical implementation enables story without dialogue—systems tell the story
- Replayability through emergent **AI Storyteller** event variations

This approach demonstrates how modern game design can create compelling narratives through systemic gameplay rather than fixed scripts, allowing player agency while maintaining dramatic structure through intelligent **procedural event generation**.

---

**Document Version:** 2.0 (GameMaster/AI Storyteller Edition)  
**Date:** November 19, 2025  
**Game Version:** Current Development Build with Procedural Storytelling Systems  
**Author:** Project Documentation  
**Keywords:** AI Storyteller, GameMaster, Procedural Storytelling, Dynamic Difficulty, Emergent Narrative, Event-Driven Design
