# Platformer Game Demo Video Script

## Introduction (1-2 minutes)

**[Show main menu or early gameplay footage]**

**Script:**
"Hello! Today I'll be demonstrating my C++ platformer game - a comprehensive 2D action platformer that combines traditional platformer mechanics with modern action-RPG elements and an innovative AI Storyteller system.

**[Brief gameplay montage showing combat, jumping, boss fight]**

### Game Overview

This game is a side-scrolling action platformer featuring:
- **Fast-paced combat** with melee attacks and elemental projectiles (fire and ice)
- **Character progression** with experience-based leveling and a skill tree system
- **Dynamic difficulty** through the GameMaster AI Storyteller - a procedural event system that orchestrates dramatic moments based on your performance
- **Boss encounters** with multi-phase battles and environmental hazards

**[Show world view]**

The game world spans 6000 pixels across 10 unique platforms. You'll fight through waves of enemies, collect experience and HP potions, unlock abilities, and ultimately face a powerful boss at the end of the world.

### Technical Highlights

What makes this project special is the implementation of 10 key computer science concepts:

**Object-Oriented Programming:**
- Deep inheritance hierarchy from Entity to Character to Player/Enemy
- Polymorphic behavior through virtual functions

**Custom Data Structures:**
- Hash Table for resource management (implemented from scratch)
- Singly and Doubly Linked Lists for game data
- Stack for game state management
- Queue for floating damage text
- N-ary Tree for the skill system

**Design Patterns:**
- Singleton pattern for game management
- Iterator pattern for data traversal
- Object pooling for projectiles and meteors

**[Show GameMaster events - rage mode or meteor fury]**

### The GameMaster AI Storyteller

A unique feature is the GameMaster system - an AI that monitors your health, combat performance, and game state to trigger procedural storytelling events:
- **Player Rage Mode** activates when you're critically injured with multiple enemies nearby
- **Boss Rage Mode** triggers at 50% boss health, escalating the difficulty
- **Meteor Fury** dramatically increases environmental hazards during desperate moments
- **Dynamic pacing** that creates unique experiences every playthrough

**[Transition to code]**

Now, let me walk you through how each of these 10 concepts is implemented in the code and how they work together to create this gameplay experience."

---

## Data Structures Overview (1-2 minutes)

**[Show a slide or screen with the table, or prepare to explain while showing code]**

**Script:**
"Before we dive into the individual implementations, let me give you an overview of the six custom data structures I built and how they're used throughout the game.

**[Display or reference the table]**

### Data Structures Summary Table

| Data Structure | Use Case | Operations | Time Complexity |
|----------------|----------|------------|-----------------|
| **Hash Table** | Resource management (texture/sound lookup by string key) | Insert, Lookup | O(1) average |
| **Singly Linked List** | Combat log messages (sequential insertion, procedural event logging) | Insert, Traverse | O(1) insert, O(n) traverse |
| **Doubly Linked List** | Projectile/potion/meteor pools (bidirectional traversal) | Insert, Remove, Traverse | O(1) at ends, O(n) search |
| **Stack (LIFO)** | Game state management (push/pop menus) | Push, Pop, Peek | O(1) |
| **Queue (FIFO)** | Floating damage text (temporal ordering of procedural events) | Enqueue, Dequeue | O(1) |
| **N-ary Tree** | Skill tree (hierarchical dependencies) | Insert, Search, Traverse | O(log n) balanced |

### Why These Structures?

**[Point to each row as you explain]**

**Hash Table:**
'I chose a hash table for the ResourceManager because I need instant access to textures and sounds by name. When the game needs to load a player sprite or play an attack sound, it can't wait - it needs O(1) average lookup time. The hash table maps string keys like 'player_idle' directly to the texture data using separate chaining for collision resolution.'

**Singly Linked List:**
'For the combat log, I need to maintain chronological order of events - oldest to newest. A singly linked list is perfect because I only insert at the tail (new events) and occasionally traverse from head to tail to display the log. I don't need to remove from the middle or traverse backwards, so the simpler singly linked structure is more efficient.'

**Doubly Linked List:**
'The projectile pool requires bidirectional traversal because I need to efficiently add and remove projectiles from both ends. When a projectile expires, I can remove it in O(1) time from anywhere in the list if I have an iterator to it. The doubly linked structure with previous pointers makes this possible.'

**Stack:**
'Game state management naturally fits a stack - when you open the pause menu, it pushes a new state on top. When you resume, it pops that state off. When you open the skill tree from the pause menu, that's another push. The LIFO ordering ensures states are exited in reverse order of entry, which is exactly what we need for nested menus.'

**Queue:**
'Floating damage numbers must appear in the order they were created - first damage dealt appears first, second damage dealt appears second. A FIFO queue guarantees this temporal ordering. When damage is dealt, it enqueues the text. The rendering system dequeues and displays them in order.'

**N-ary Tree:**
'The skill tree has hierarchical dependencies - you can't unlock 'Ice Projectile Level 2' without first unlocking 'Ice Projectile Level 1'. An N-ary tree naturally represents this parent-child relationship. Each skill node can have multiple children (skills that unlock after it), and I can traverse the tree to find unlockable skills based on what the player has already learned.'

### Performance Considerations

**[Emphasize this point]**

'Notice that all the core operations - insert, lookup, push, pop, enqueue, dequeue - are O(1) constant time. This is critical for a real-time game running at 60 frames per second. If resource lookup was O(n), the game would lag every time it needed to load a texture. If state transitions were slow, menus would feel sluggish. Choosing the right data structure for each use case ensures the game runs smoothly.'

**[Transition]**

Now let's look at each implementation in detail, starting with Object-Oriented Programming."

---

## 1. OBJECT-ORIENTED PROGRAMMING

### 1a. Inheritance and Derived Classes (2-3 minutes)

**[Show gameplay with player and enemies]**

**Script:**
"First, let's talk about Inheritance. My game uses a class hierarchy to organize all game entities."

**[Open Entity.h]**
"At the base, I have the `Entity` class which provides common functionality for all game objects. It contains:
- Position and velocity vectors (lines 9-10)
- A sprite for rendering (line 8)
- An active state flag (line 11)
- A pure virtual `update()` method (line 20) that forces all derived classes to implement their own update logic"

**[Open Character.h]**
"The `Character` class inherits from `Entity` (line 5) and adds combat-specific features:
- HP (health points) and maxHP (lines 8-9)
- Damage values (line 10)
- Movement speed (line 12)
- Ground detection for jumping (line 11)
- Virtual methods like `takeDamage()` (line 21) and `isAlive()` (line 23) that can be overridden"

**[Open Player.h]**
"The `Player` class extends `Character` (line 14) and adds:
- Animation system with idle, run, and attack animations (lines 31-34)
- Input handling for keyboard controls (line 88)
- Jump mechanics with double-jump capability (lines 56-57)
- Special attack with dash movement (lines 45-48)
- Experience and leveling system (lines 72-74)
- Skill tree (line 69) and inventory for HP potions (lines 75-76)"

**[Open Enemy.h and Boss.h]**
"Similarly, `Enemy` inherits from `Character` (line 20) and adds:
- AI behavior with patrol, chase, and attack states (lines 12-17, 34)
- Detection and attack ranges (lines 37-38)
- Status effects like ice stacks (lines 43-44) and burning (lines 46-49)
- The `Boss` class (Boss.h line 19) further extends `Enemy` with boss-specific mechanics like spawning, regeneration, and multiple attack patterns"

**[Show Projectile.h, FireProjectile.h, IceProjectile.h]**
"I also have a projectile hierarchy:
- `Projectile` base class (Projectile.h line 5) with damage and lifetime
- `FireProjectile` (FireProjectile.h line 7) applies burn damage over time
- `IceProjectile` (IceProjectile.h line 7) slows and freezes enemies
Both inherit from `Entity` and override the `update()` method"

**Key Points to Emphasize:**
- "This demonstrates the IS-A relationship: Player IS-A Character IS-A Entity"
- "Code reuse through inheritance reduces duplication"
- "Each level adds more specific functionality"

---

### 1b. Polymorphism (2-3 minutes)

**[Show gameplay with different projectiles and enemies]**

**Script:**
"Now let's look at Polymorphism - the ability to treat derived classes through their base class interface."

**[Open relevant code showing virtual functions]**
"Notice how all my classes have virtual functions:
- `Entity` has `virtual void update(float dt) = 0` (Entity.h line 20) - a pure virtual function
- `Character` has `virtual void takeDamage(int dmg)` (Character.h line 21) and `virtual bool isAlive()` (line 23)
- Each derived class provides its own implementation"

**[Show Enemy.h]**
"In my game loop, I can store different entity types in collections and call their methods polymorphically:
- When I call `update()` on a Player pointer, it executes Player's update logic
- When I call `update()` on an Enemy pointer, it executes Enemy's AI logic (Enemy.h line 63)
- The correct version is determined at runtime through virtual function dispatch"

**[Show FireProjectile and IceProjectile]**
"For projectiles:
- `FireProjectile::update()` handles fire animation and applies burn effects
- `IceProjectile::update()` handles ice animation and applies freeze effects
- Both can be stored as `Projectile*` pointers and updated polymorphically"

**Key Points to Emphasize:**
- "Runtime polymorphism through virtual functions"
- "Allows treating different types uniformly through base class pointers"
- "Each object knows its own type and executes the correct method"

---

## 2. COMPOSITE DATA STRUCTURES

### 2a. Hash Tables - Independent Research (3-4 minutes)

**[Show ResourceManager in action - textures loading]**

**Script:**
"For my independent research, I implemented a Hash Table from scratch to manage game resources efficiently."

**[Open HashTable.h]**
"My `HashTable` (line 15) is a templated class that uses separate chaining for collision resolution. Let me explain the key components:

**Structure:**
- An array of bucket pointers (line 32) - dynamic array
- Each bucket is a linked list of key-value pairs (Node struct, lines 19-30)
- Uses `std::hash` to compute hash values
- Automatically resizes when load factor exceeds 0.75 (line 35)"

**[Scroll through the implementation]**
"Key features I implemented:

1. **Hash Function** (line 117): Uses `std::hash<K>` and modulo to map keys to buckets
   ```cpp
   std::hash<K> hasher;
   return hasher(key) % capacity;
   ```

2. **Insert Operation** (line 240): 
   - Computes hash index (line 242)
   - Searches the chain for existing key (lines 246-255)
   - Either updates existing value or creates new node at bucket head (lines 258-261)
   - Triggers resize if load factor exceeds threshold (lines 264-267)

3. **Resize Operation** (line 125):
   - Doubles capacity (line 130)
   - Rehashes all existing entries into new table (lines 141-153)
   - Maintains O(1) average case for operations

4. **Search/Get** (line 304): Traverses the chain to find matching key (lines 310-320)

5. **Remove** (line 383): Handles removal from head, middle, or tail of chain (lines 390-414)"

**[Open ResourceManager.h]**
"I use hash tables in my `ResourceManager` to store:
- Textures mapped by name (line 88): `HashTable<std::string, std::unique_ptr<sf::Texture>>`
- Animations mapped by name (line 89): `HashTable<std::string, std::unique_ptr<Animation>>`

This gives me O(1) average-case lookup time for loading sprites and animations during gameplay."

**[Mention diagnostic methods]**
"I also implemented diagnostic methods:
- `loadFactor()` (line 502): current load factor
- `bucketCount()` (line 495): number of buckets
- `longestChainLength()` (line 548): worst-case chain length
- `averageChainLength()` (line 574): average chain length for collision analysis"

**Key Points to Emphasize:**
- "Separate chaining handles collisions with linked lists"
- "Dynamic resizing maintains O(1) average performance"
- "Real-world application: fast resource lookup by name"
- "Implemented copy/move semantics for proper memory management"

---

### 2b. Singly Linked List (2 minutes)

**[Show combat log or relevant gameplay]**

**Script:**
"I implemented a Singly Linked List to store combat log entries in chronological order."

**[Open SinglyLinkedList.h]**
"My implementation includes:

**Structure** (line 14):
- Node struct with data and next pointer (lines 14-21)
- Head pointer (line 23) - oldest element
- Tail pointer (line 24) - newest element
- Count variable for size tracking (line 25)

**Operations:**
- `pushBack()` (line 136): Adds element at tail in O(1) - maintains chronological order
  - Creates new node (line 138)
  - Links to tail or sets as first element (lines 140-148)
  
- `pushFront()` (line 153): Adds element at head in O(1)

- `popFront()` (line 166): Removes oldest element in O(1)
  - Saves current head (line 173)
  - Advances head pointer (line 174)
  - Deletes old node (line 181)

- `removeAt()` (line 186): Removes element at specific index in O(n)

- `removeIf()` (line 220): Removes all elements matching a predicate - useful for filtering old log entries

- `forEach()` (line 264): Applies a function to each element for iteration"

**[Show the copy-and-swap idiom]**
"I implemented the Rule of Five:
- Copy constructor (line 86)
- Copy assignment using copy-and-swap idiom (line 92)
- Move constructor (line 103)
- Move assignment (line 113)
- Destructor (line 130)"

**Key Points to Emphasize:**
- "Efficient O(1) insertion at both ends with tail pointer"
- "Used for maintaining chronological order of events"
- "Template-based for type flexibility"

---

### 2c. Doubly Linked List (2 minutes)

**[Show relevant gameplay feature]**

**Script:**
"I also implemented a Doubly Linked List which allows bidirectional traversal."

**[Open DoublyLinkedList.h]**
"Key differences from singly linked list:

**Node Structure** (line 11):
- Each node has both `next` (line 19) AND `previous` (line 20) pointers
- Allows traversal in both directions
- Uses a NIL sentinel node (line 15) to represent empty/end

**Advantages:**
- `popBack()` (line 414): Can remove from tail in O(1) - impossible with singly linked list
  - Saves tail node (line 421)
  - Moves tail to previous (line 422)
  - Updates links (lines 425-428)
  
- `erase()` (line 438): Can remove any element in O(1) if you have an iterator to it
  - Updates previous and next links (lines 450-459)
  
- Bidirectional iteration through custom iterator"

**[Open DoublyLinkedNodeIterator.h]**
"I implemented a custom iterator (line 13) that supports:
- Forward iteration with `operator++` (line 85)
- Backward iteration with `operator--` (line 104)
- Dereference with `operator*` (line 67)
- Arrow operator `operator->` (line 76)
- This makes it compatible with range-based for loops"

**[Show forEach implementation]**
"The `forEach()` method (DoublyLinkedList.h line 516) demonstrates the Iterator pattern in action - it traverses the list and applies a function to each element."

**Key Points to Emphasize:**
- "Bidirectional traversal capability"
- "O(1) removal from both ends"
- "Custom iterator for STL-like interface"
- "NIL sentinel pattern eliminates null checks"

---

## 3. ABSTRACT DATA TYPES

### 3a. Stack (2-3 minutes)

**[Show game state transitions or relevant feature]**

**Script:**
"I implemented a Stack ADT to manage game states - like menu, playing, paused, game over."

**[Open Stack.h]**
"My Stack follows the LIFO (Last-In-First-Out) principle:

**Structure** (line 36):
- Node struct with data and next pointer (lines 36-46)
- topNode pointer (line 48)
- count for size tracking (line 49)

**Core Operations:**

1. `push()` (line 143): Adds element to top in O(1)
   - Creates new node (line 145)
   - Links it to current top
   - Updates top pointer (line 146)
   - Calls debug logger (line 148)

2. `pop()` (line 163): Removes top element in O(1)
   - Checks if empty (lines 165-166)
   - Saves old top (line 168)
   - Moves top pointer down (line 169)
   - Calls debug logger (line 170)
   - Deletes old node (line 171)

3. `top()` (line 177): Returns reference to top element without removing
   - Throws if empty (lines 179-180)
   - Returns topNode->data (line 182)

4. `peek()` (line 197): Access elements deeper in stack by level
   - Traverses down to specified level (lines 205-206)
   - Returns element at that depth"

**[Show StackDebugLogger specialization]**
"I created a debug logger (line 21) that prints when game states are pushed/popped:
- Template specialization for GameStateData (lines 20-25)
- Helps track state transitions
- Demonstrates template specialization technique"

**Key Points to Emphasize:**
- "LIFO ordering enforces proper state management"
- "O(1) operations for push, pop, and top"
- "Template-based for storing any type"
- "Move semantics enabled, copy disabled for efficiency"

---

### 3b. Queue (2 minutes)

**[Show floating damage text appearing]**

**Script:**
"I implemented a Queue ADT to manage floating damage text in FIFO order."

**[Open Queue.h]**
"My Queue follows the FIFO (First-In-First-Out) principle:

**Structure** (line 47):
- Node struct with data and next pointer (lines 47-57)
- frontNode pointer (line 59) - oldest element
- rearNode pointer (line 60) - most recently inserted
- count for size tracking (line 61)

**Core Operations:**

1. `enqueue()` (line 74): Adds element to rear in O(1)
   - Creates new node (line 76)
   - If empty, sets both front and rear (lines 78-81)
   - Otherwise, links to rear and updates rear pointer (lines 83-86)
   - Increments count and logs (lines 88-89)

2. `dequeue()` (line 112): Removes element from front in O(1)
   - Checks if empty (lines 114-115)
   - Saves front node (line 117)
   - Moves front pointer forward (line 118)
   - Updates rear if queue becomes empty (lines 121-122)
   - Logs and deletes node (lines 124-126)

3. `front()` (line 130): Returns reference to front element
   - Throws if empty (lines 132-133)
   - Returns frontNode->data (line 135)

4. `forEach()` (line 168): Iterates through all elements
   - Traverses from front to rear (lines 170-175)

5. `removeIf()` (line 184): Removes elements matching predicate
   - Handles removal from front, middle, or rear (lines 186-224)"

**[Show QueueDebugLogger specialization]**
"I specialized the debug logger (line 21) for `std::unique_ptr<FloatingText>`:
- Prints the damage text when enqueued/dequeued (lines 23-35)
- Helps debug the damage display system"

**Key Points to Emphasize:**
- "FIFO ordering ensures damage text appears in correct order"
- "O(1) enqueue and dequeue operations"
- "Used for managing temporary visual effects"

---

### 3c. Tree (2-3 minutes)

**[Show skill tree UI or relevant feature]**

**Script:**
"I implemented an N-ary Tree ADT for the skill tree system."

**[Open Tree.h]**
"My `NTree` (line 10) is a generic N-ary tree where each node can have up to N children:

**Structure:**
- Template parameters: `T` for data type, `N` for max children (line 10)
- Key value stored at node (line 13)
- Fixed-size array of N child pointers (line 14)
- Parent pointer for upward traversal (line 15)
- NIL sentinel node (line 19) representing empty children

**Key Features:**

1. `attachNTree()` (line 150): Attaches a subtree as a child
   - Validates index and checks for empty slot (lines 152-157)
   - Updates child pointer (line 159)
   - Updates parent pointer (lines 160-161)

2. `detachNTree()` (line 167): Removes and returns a child subtree
   - Saves detached pointer (line 174)
   - Replaces with NIL (line 175)
   - Clears parent pointer (lines 177-178)

3. `search()` (line 202): Recursively searches for a key using depth-first traversal
   - Returns nullptr if empty (lines 204-205)
   - Checks current node (lines 207-208)
   - Recursively searches children (lines 211-216)

4. `getSize()` (line 188): Recursively counts all nodes in subtree
   - Returns 0 for NIL (lines 190-191)
   - Counts this node plus all children (lines 193-196)

5. `traverseLevelOrder()` (line 224): Breadth-first traversal using internal queue
   - Implements minimal queue internally (lines 230-278)
   - Visits nodes level by level (lines 283-297)
   - Applies visitor function to each node's data"

**[Show the level-order traversal implementation]**
"The level-order traversal is interesting:
- I implemented a minimal internal queue (line 230)
- Visits nodes level by level
- Applies a visitor function to each node's data (line 289)
- Useful for rendering skill tree UI layer by layer"

**Key Points to Emphasize:**
- "N-ary tree allows multiple children per node"
- "NIL sentinel pattern simplifies edge cases"
- "Supports both depth-first (search) and breadth-first (level-order) traversal"
- "Parent pointers enable upward navigation"

---

## 4. DESIGN PATTERNS

### 4a. Iterator Pattern (2 minutes)

**[Show code demonstrating iteration]**

**Script:**
"I implemented the Iterator Pattern to provide a standard way to traverse my data structures."

**[Open DoublyLinkedNodeIterator.h]**
"My custom iterator for doubly linked lists (line 13):

**Interface:**
- `operator*()` (line 67): Dereferences to get value reference
  ```cpp
  return node->getValueRef();
  ```

- `operator->()` (line 76): Arrow operator for member access
  ```cpp
  return &node->getValueRef();
  ```

- `operator++()` (line 85): Moves to next element
  ```cpp
  node = node->getNext();
  return *this;
  ```

- `operator--()` (line 104): Moves to previous element
  ```cpp
  node = node->getPrevious();
  return *this;
  ```

- `operator==` and `operator!=` (lines 121, 128): Comparison operators
  ```cpp
  return node == other.node;
  ```"

**[Show begin() and end() methods in DoublyLinkedList.h]**
"The list provides `begin()` (line 316) and `end()` (line 323) methods that return iterators:
- `begin()` returns iterator to first element
- `end()` returns iterator to NIL sentinel
- This enables range-based for loops:
  ```cpp
  for (auto& item : myList) {
      // process item
  }
  ```"

**[Show forEach() implementations]**
"I also implemented `forEach()` methods in multiple data structures:
- `SinglyLinkedList::forEach()` (line 264)
- `DoublyLinkedList::forEach()` (line 516)
- `Queue::forEach()` (line 168)
- `NTree::traverseLevelOrder()` (line 224)

These accept function objects and apply them to each element:
```cpp
list.forEach([](auto& element) {
    // Process element
});
```"

**Key Points to Emphasize:**
- "Provides uniform interface for traversing different data structures"
- "Enables range-based for loops and STL-like usage"
- "Separates iteration mechanism from element processing"
- "Both external iterator (DoublyLinkedNodeIterator) and internal iterator (forEach) patterns"

---

### 4b. Singleton Pattern (2 minutes)

**[Show game initialization]**

**Script:**
"Finally, I implemented the Singleton Pattern to ensure only one Game instance exists."

**[Open Game.h]**
"The Singleton Pattern ensures a class has only one instance and provides global access to it:

**Implementation:**
- `static Game& getInstance()` (line 13): Returns the single instance
- Private constructor (line 28): Prevents direct instantiation
- Deleted copy constructor and assignment operator (lines 18-19): Prevents copying
- Deleted move constructor and assignment operator (lines 20-21): Prevents moving"

**[Open Game.cpp]**
"The `getInstance()` method (line 6):
```cpp
Game& Game::getInstance() {
    static Game instance;  // Created once, lives for program duration
    return instance;
}
```
- Uses a static local variable
- Guaranteed to be initialized only once
- Thread-safe in C++11 and later
- Lazy initialization - created on first use"

**[Open main.cpp]**
"In main.cpp (lines 12-13):
```cpp
auto& g1 = Game::getInstance();
auto& g2 = Game::getInstance();
// g1 and g2 are the same object
```
Both references point to the same Game instance."

**Why Singleton for Game?**
- Only one game should run at a time
- Provides global access point for game state
- Manages window, resources, and game loop
- Prevents accidental multiple game instances

**Key Points to Emphasize:**
- "Ensures single instance throughout program lifetime"
- "Global access point through getInstance()"
- "Private constructor prevents external instantiation"
- "Deleted copy/move operations prevent duplication"
- "Thread-safe initialization with C++11 static locals"

---

## Conclusion (1 minute)

**[Show final gameplay - boss battle or victory screen]**

**Script:**
"In summary, I've demonstrated how 10 fundamental computer science concepts work together to create a complete, playable action platformer game.

### Technical Implementation Recap:

**1. Object-Oriented Programming**: 
   - Inheritance hierarchy from Entity → Character → Player/Enemy/Boss
   - Polymorphic behavior through virtual functions enabling different entity types to coexist
   - Code reuse and extensibility through proper class design

**2. Composite Data Structures**: 
   - Custom Hash Table implementation for O(1) resource lookup
   - Singly Linked List for chronological combat log entries
   - Doubly Linked List with custom iterator for bidirectional traversal

**3. Abstract Data Types**: 
   - Stack (LIFO) for game state management - menus, pause, gameplay
   - Queue (FIFO) for floating damage text temporal ordering
   - N-ary Tree for hierarchical skill system with dependencies

**4. Design Patterns**: 
   - Iterator pattern for uniform data structure traversal
   - Singleton pattern ensuring single game instance
   - Object pooling for efficient projectile and meteor management

### How They Create the Game:

**[Show gameplay examples as you mention each]**

These concepts aren't just academic exercises - they're essential to the game's functionality:

- The **Hash Table** enables instant texture and animation loading during gameplay
- The **inheritance hierarchy** lets me treat all entities uniformly while maintaining unique behaviors
- The **Stack** manages seamless transitions between menus and gameplay
- The **Queue** ensures damage numbers appear in the correct order
- The **Tree** structure naturally represents skill dependencies and unlocking paths
- The **GameMaster AI Storyteller** uses these data structures to orchestrate procedural events - tracking game state in real-time and triggering dramatic moments like rage modes and meteor fury

### Project Scope:

This project represents:
- **8,000+ lines** of C++ code
- **50+ source files** with clear separation of concerns
- **6 custom data structures** implemented from scratch
- **Multiple design patterns** applied appropriately
- **Real-time game systems** including physics, collision, AI, and procedural storytelling
- **Complete game loop** from main menu to victory screen

**[Show final victory or impressive gameplay moment]**

The result is a fully functional action platformer that demonstrates both solid computer science fundamentals and practical game development skills. Every system works together - from the low-level data structures managing resources and game state, to the high-level AI Storyteller creating dynamic, emergent gameplay experiences.

Thank you for watching this demonstration!"

---

## Recording Tips

### Preparation:
- Have all relevant files open in tabs for quick switching
- Set up screen recording to capture both gameplay and code
- Test audio quality beforehand
- Practice transitions between gameplay and code
- Prepare a simple outline/cheat sheet with file names and line numbers

### During Recording:
- Speak clearly and at moderate pace
- Use cursor/highlighter to point at specific code lines
- Show gameplay examples before diving into code
- Pause briefly between sections
- If you make a mistake, pause and continue - you can edit later

### Code Highlighting:
- Zoom in on code so it's readable (increase font size)
- Highlight key lines (class declarations, virtual functions, etc.)
- Scroll slowly through implementations
- Show both .h and .cpp files where relevant
- Use IDE features like "Go to Definition" to show connections

### Gameplay Footage:
- Show features in action before explaining code
- Demonstrate polymorphism with different enemy types
- Show resource loading for hash table
- Display floating text for queue
- Show skill tree for tree structure
- Demonstrate state transitions for stack

### Time Management:
- **Total video**: 20-25 minutes
- **Introduction**: 30 seconds
- **OOP**: 4-6 minutes (most important)
- **Data Structures**: 7-9 minutes (hash table gets most time)
- **ADTs**: 6-7 minutes
- **Design Patterns**: 4 minutes
- **Conclusion**: 30 seconds

### Visual Layout:
- Split screen: gameplay on one side, code on the other
- Or alternate: show gameplay, then switch to code
- Use picture-in-picture for your webcam (optional)
- Add text overlays for section titles

### Common Mistakes to Avoid:
- Don't rush through code - give viewers time to read
- Don't assume knowledge - explain technical terms
- Don't skip the "why" - explain why you chose each approach
- Don't forget to show the code actually running in the game

Good luck with your demo! 🎮
