## 4.2 Singleton Design Pattern

### 4.2.1 Application

**Application Area:** `Game` singleton as the top-level controller for a 2D platformer.

In this platformer, the `Game` class is responsible for owning and driving the entire application:

- It owns the main game loop entry point (`run()`).
- It manages the current `Session`, which in turn owns the `GameWorld`, camera, physics, input, combat system, and game states.
- It owns a single `ResourceManager` instance that is shared with the `Session` and, indirectly, with the rest of the game.

From the code:

- `main.cpp` calls `Game::getInstance().run();`
- `Game` has a private constructor and a static `getInstance()` method (Singleton).
- `Game` holds:
  - `ResourceManager resourceManager;`
  - `std::unique_ptr<Session> session;`
  - `bool running;`

**Desired Operation (Detailed):**

- When the executable starts, only one `Game` object is ever created:
  - `Game::getInstance()` returns a reference to a single static `Game` instance.
  - No other part of the code can create or copy a `Game` object (constructors and assignment operators are deleted).
- `Game::run()`:
  - Sets `running = true`.
  - Calls `newGame()` to create a new `Session` using the shared `resourceManager`.
  - Hands control to `Session::run()` which manages the window, game loop, and state transitions.
  - After the session ends, cleans up and sets `running = false`.
- `Game::newGame()`:
  - Creates a new `Session` (`session = std::make_unique<Session>(resourceManager);`).
  - Ensures that the `ResourceManager` instance owned by `Game` is the one passed into the new session.
- `Game::exit()`:
  - Sets `running = false`.
  - Resets the `session` pointer so all sub-systems are cleaned up in a controlled order.

The key requirements for this application are:

- There must be exactly one main game controller, owning exactly one main loop.
- All major systems (session, world, resources) should be created, run, and destroyed under this single controller.
- The entry point (`main`) should have a simple, clear call like `Game::getInstance().run();` instead of manually juggling multiple global objects.

This makes `Game` a natural fit for the Singleton design pattern in this platformer.

### 4.2.2 Concept

**How Singleton Matches the Desired Operation:**

- **Single Instance Guarantee:**

  - The Singleton pattern ensures that only one `Game` instance exists.
  - This is important because the platformer should have only one main game loop, one active `Session`, and one top-level owner of the window and core systems.

- **Global Access Point:**

  - `Game::getInstance()` provides a simple, globally known entry point.
  - The `main` function, or any other top-level code, can obtain the `Game` instance without passing it through multiple layers of constructors or global variables.

- **Centralized Lifecycle Management:**

  - The `Game` singleton is responsible for creating and destroying the `Session` at the correct times.
  - It owns the `ResourceManager` and passes it into the `Session`, ensuring consistent resource usage.
  - Initialization and shutdown logic (including error handling in `run()`) are centralized in one place.

- **Consistency of Global State:**
  - Global game-wide concepts such as “is the game currently running?” (`running` flag) are tracked in a single location.
  - Because there is only one `Game` instance, there is no risk of multiple windows or multiple independent sessions conflicting with each other.

Overall, the Singleton pattern maps directly to the idea of a unique, global `Game` object, which is the conceptual “root” of the entire platformer and the owner of the main loop and core subsystems.

**Alternative Structures and Why They Are Less Preferred Here:**

1. **Free Functions and Raw Globals:**

   - Example: Having a global `Game` variable and free functions like `runGame()`, `startSession()`, etc.
   - Pros:
     - Very simple to write.
   - Cons:
     - Global variables make initialization order and lifetime harder to control, especially across multiple translation units.
     - Logic and state can become scattered (game state in one global, window in another, resources in a third).
     - Harder to encapsulate behavior and enforce invariants.
   - Conclusion:
     - Works for tiny prototypes, but the Singleton `Game` class provides much better encapsulation and clearer ownership in a multi-class platformer.

2. **Stack-Allocated `Game` in `main` (Non-Singleton):**

   - Example: `int main() { Game game; game.run(); }`
   - Pros:
     - Clear lifetime: `Game` is created and destroyed in `main`.
     - No static/global state needed for `Game` itself.
   - Cons:
     - If other parts of the code ever need access to the `Game` (for example, to query `isRunning()` or to trigger a new session), they must receive a reference or pointer from `main`.
     - This can add boilerplate and may not match the desired “single global controller” style in the design brief.
   - Conclusion:
     - Architecturally clean, but requires more plumbing. The Singleton approach trades some strict architectural purity for a simple and clear one-line entry (`Game::getInstance().run();`) that is suitable for this assignment.

3. **Service Locator for the Game Controller:**

   - Example: A `ServiceLocator` that stores a pointer to the current `Game` instance.
   - Pros:
     - Decouples code from the concrete `Game` type if designed carefully.
   - Cons:
     - Adds an extra indirection layer and complexity.
     - Still effectively introduces a global access point; in a small/medium student platformer the extra abstraction is not necessary.
   - Conclusion:
     - Better suited for large engines with many interchangeable services. For this project, directly using a `Game` Singleton is clearer.

4. **Full Dependency Injection (DI) Setup:**
   - Example: Having a composition root that wires together `Game`, `Session`, `ResourceManager`, etc., and passes references everywhere explicitly.
   - Pros:
     - Very testable and flexible; promotes loose coupling.
   - Cons:
     - Overkill for a single-player 2D platformer with a relatively small codebase.
     - Adds a lot of boilerplate and complexity to an educational project.
   - Conclusion:
     - A good approach in larger systems, but the Singleton `Game` is a pragmatic and adequate solution here.

In summary, while globals, non-singleton `Game` objects, service locators, or full DI could be used, the Singleton pattern is preferred in this platformer because it:

- Explicitly models the intent of “exactly one `Game` controller.”
  -,Encapsulates the main loop, session management, and resource ownership in a single coherent type.
- Provides a simple, expressive entry point (`Game::getInstance().run();`) that matches the design of this project.

### 4.2.3 Implementation & Output

This section lists the key files and code fragments that implement the `Game` Singleton in the platformer.

**File 1: `main.cpp` – Entry Point Using the Singleton**

- **Purpose:** Provides the application entry point and starts the game via the `Game` singleton.
- **Key Code Fragment:**
  - Includes the `Game` header.
  - Calls `Game::getInstance().run();` inside a `try`/`catch` block.

```cpp
#include "Game.h"

int main()
{
    try
    {
        Game::getInstance().run();
    }
    catch (const std::exception &e)
    {
        // error reporting
        return 1;
    }

    return 0;
}
```

**File 2: `Game.h` – Singleton `Game` Class Declaration**

- **Purpose:** Declares the `Game` class as a Singleton and defines its public interface.
- **Key Code Fragments:**
  - `static Game &getInstance();` – global access point.
  - Private constructor and destructor.
  - Deleted copy/move operations to prevent multiple instances.
  - Member fields that the singleton owns (`ResourceManager`, `Session`, `running` flag).

```cpp
class Game
{
public:
    static Game &getInstance();

    void run();
    void newGame();
    void exit();

    bool isRunning() const;

private:
    Game();
    ~Game();

    Game(const Game &) = delete;
    Game &operator=(const Game &) = delete;
    Game(Game &&) = delete;
    Game &operator=(Game &&) = delete;

    ResourceManager resourceManager;
    std::unique_ptr<Session> session;
    bool running;
};
```

**File 3: `Game.cpp` – Singleton Definition and Core Behavior**

- **Purpose:** Implements the singleton accessor and the main control flow for the platformer.
- **Key Code Fragments:**
  - `Game &Game::getInstance()` defines the function-local static instance (Singleton).
  - `Game::Game()` constructor initializes `resourceManager`, `session`, and `running`.
  - `Game::run()` wraps the core loop in error handling.
  - `Game::newGame()` creates a new `Session` using the shared `resourceManager`.
  - `Game::exit()` and `~Game()` handle cleanup and session destruction.

```cpp
Game &Game::getInstance()
{
    static Game instance;
    return instance;
}

Game::Game()
    : resourceManager(), session(nullptr), running(false) {}

Game::~Game()
{
    session.reset();
}

void Game::run()
{
    try
    {
        running = true;
        newGame();
        running = false;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Game Error: " << e.what() << std::endl;
        running = false;
    }
}

void Game::newGame()
{
    session = std::make_unique<Session>(resourceManager);
    session->run();
}

void Game::exit()
{
    running = false;

    if (session)
    {
        session.reset();
    }
}
```

Together, these three files show:

- Where the singleton is accessed (`main.cpp`).
- How the singleton is declared (`Game.h`).
- How the singleton instance is created and used to control the platformer’s main loop and session (`Game.cpp`).

### 4.2.4 Troubleshooting Summary

**Issues Encountered During Implementation:**

- **Ensuring a True Singleton `Game`:**

  - Needed to prevent accidental copies or extra instances of `Game`.
  - Required deleting copy/move constructors and assignment operators, and making the constructor private.

- **Initialization Order and Resource Ownership:**

  - `Game` owns `ResourceManager` and passes it to `Session`.
  - Had to ensure that `ResourceManager` is constructed before the `Session` and is still valid for the entire lifetime of the `Session`.

- **Session Lifetime Management:**

  - Ensuring `session` is properly created in `newGame()` and cleaned up in the destructor or `exit()`.
  - Avoiding dangling pointers or accessing the `Session` after it has been reset.

- **Error Handling in the Main Loop:**

  - `Game::run()` wraps the main flow in a `try`/`catch` block.
  - Needed to make sure that if any exception occurs inside `Session::run()`, the error is logged and `running` is set to `false` so the program exits cleanly.

- **Testability and Coupling:**
  - Because `Game` is globally accessible via `getInstance()`, code can become tightly coupled to the singleton.
  - This can make unit testing harder, as tests might need to interact with or reset the global `Game` state.

**How the Problems Were Solved:**

- **Implementing the Singleton Correctly:**

  - Implemented `Game::getInstance()` using a function-local static:
    - `static Game instance; return instance;`
  - Made the constructor and destructor private, and deleted copy/move operations to enforce a single instance.

- **Clear Ownership and Lifetime:**

  - Defined `ResourceManager resourceManager;` and `std::unique_ptr<Session> session;` as members of `Game`, ensuring their lifetimes are tied to the `Game` singleton.
  - Created the `Session` inside `newGame()` using the existing `resourceManager`, guaranteeing that the same resource manager is used throughout a session.

- **Safe Creation and Cleanup of `Session`:**

  - Used `std::make_unique<Session>(resourceManager);` to create the session.
  - In the destructor and `exit()`, called `session.reset();` to explicitly destroy the session and all owned sub-systems in a deterministic order.

- **Robust Error Handling:**

  - Wrapped `run()` in `try`/`catch` to catch any exceptions thrown during the game loop.
  - Logged errors to `std::cerr` and ensured `running` is set to `false` so the program terminates safely instead of leaving the singleton in an inconsistent state.

- **Managing Coupling in a Small Project:**
  - Limited direct access to `Game::getInstance()` mostly to the entry point and top-level flow control.
  - Kept most game logic inside `Session`, `GameWorld`, and other subsystems so that they can be reasoned about largely independently of the singleton.

**Resources / References Used:**

- Gamma, E., Helm, R., Johnson, R., & Vlissides, J. (1994). _Design Patterns: Elements of Reusable Object-Oriented Software_. Addison-Wesley — canonical reference for the Singleton pattern.
- Refactoring.Guru. “Singleton” — conceptual explanation and common implementation variants: `https://refactoring.guru/design-patterns/singleton`
- C++ references for static local variables and object lifetime (e.g., cppreference.com) to confirm that the function-local static in `getInstance()` is initialized in a thread-safe way in modern C++.
- SFML documentation for window and resource management patterns, to ensure the `Game` singleton fits well with SFML’s recommendations.

**Use of GenAI (GPT) in Completing This Concept:**

- **Did you use GenAI?**

  - Yes. Generative AI (GPT) was used to help structure and refine the written explanation for how the Singleton pattern is applied to the `Game` class in this platformer.

- **Prompts Used (Examples):**

  - “Write a Detailed Report Section for Singleton Design Pattern, containing 4.2 Singleton Design Pattern, 4.2.1 Application, 4.2.2 Concept, 4.2.3 Implementation & Output, 4.2.4 Troubleshooting Summary.”
  - “Write it for my platformer game” (to adapt the explanation to the actual `Game` and `Session` structure).
  - Follow-up prompts to focus the explanation on the `Game` singleton instead of `ResourceManager`.

- **Snapshots of Key GPT Responses:**
  - Screenshots of the relevant GPT conversation should be captured by the student/user and embedded here, for example:
    - `![GPT Response – Singleton Concept](images/gpt-singleton-concept.png)`
    - `![GPT Response – Troubleshooting Summary](images/gpt-singleton-troubleshooting.png)`
  - (These image paths are placeholders; replace them with actual screenshot paths in your project.)

**Cited Resources / References:**

- Gamma, E., Helm, R., Johnson, R., & Vlissides, J. (1994). _Design Patterns: Elements of Reusable Object-Oriented Software_. Addison-Wesley.
- Refactoring.Guru. “Singleton.” Available at: `https://refactoring.guru/design-patterns/singleton`
- C++ reference documentation for static local variables and object lifetime (e.g., `https://en.cppreference.com/`).
- SFML official documentation and tutorials for structuring game loops and resource ownership in C++/SFML projects.
