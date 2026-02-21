# Realm of Valor

A 2D side-scrolling action platformer built in C++ with SFML 2.5.1, developed as a Data Structures programming project at Swinburne University. The game puts every major data structure you learn in class to actual use — the skill tree runs on an N-ary tree, the combat log uses a singly linked list, key bindings live in a hash table, and so on.

---

## Gameplay Overview

You play as a warrior working through a side-scrolling world, fighting waves of enemies that get progressively harder, and eventually facing a boss. The combat system has a bit of depth to it:

- **Melee** — basic close-range attack
- **Projectiles** — switch between Fire and Ice mid-combat; fire applies burn stacks, ice stacks build toward a freeze
- **Special attack** — a chargeable dash strike with its own cooldown
- **HP Potions** — scattered around the world, collected on contact
- **Rage mode** — activates automatically below 30% HP when enemies are nearby, boosting damage for 8 seconds
- **Skill tree** — earn levels, spend points to unlock and upgrade fire/ice/special abilities
- **Save system** — two save slots, persisted to disk; your level, stats, and unlocked skills carry over

Enemies scale dynamically based on how fast you clear the early waves. Clear them quickly and the later ones hit harder; take your time and they're slightly nerfed. The boss enters at 50% HP rage, gaining speed and attack rate.

Meteors also fall periodically throughout the run, starting sparse and ramping up the longer the fight goes. When the boss dies, there's a brief slow-motion effect before the win screen.

---

## Data Structures Used

This was written for a Data Structures course, so each structure is implemented from scratch (no STL containers for the core game logic):

| Structure             | Where it's used                                                     |
| --------------------- | ------------------------------------------------------------------- |
| `HashTable<K, V>`     | Key binding storage — maps `GameAction` → `sf::Keyboard::Key`       |
| `NTree<T, N>`         | Skill tree — each node is a skill, children are unlockable upgrades |
| `DoublyLinkedList<T>` | Active entity pool management                                       |
| `SinglyLinkedList<T>` | Combat event log                                                    |
| `Stack<T>`            | Game state stack (main menu → gameplay → pause)                     |
| `Queue<T>`            | Pending damage/effect queue                                         |

All of these are in `src/data_structures/` as header-only templates.

---

## Design Patterns

- **Singleton** — `Game::getInstance()` enforces a single game instance; address equality is printed on startup as a runtime proof
- **Inheritance & Polymorphism** — `Entity → Character → Player / Enemy / Boss`, `Projectile → FireProjectile / IceProjectile`
- **Iterator** — custom iterator on `DoublyLinkedList` for range-based traversal

---

## Project Structure

```
programming-project/
├── main.cpp
├── build_and_run.bat       # Main build + run script (Windows)
├── build.ps1               # PowerShell alternative
├── keybindings.cfg         # Default key bindings
├── assets/                 # Textures, fonts, sounds
├── bin/                    # Compiled output (main.exe goes here)
│   └── saves/              # Save files (savegame1.txt, savegame2.txt)
├── SFML-2.5.1/             # SFML headers and libs (bundled)
└── src/
    ├── core/               # Game loop, session, state machine, constants
    ├── entities/           # Player, Enemy, Boss, projectiles, animations
    ├── systems/            # Physics, combat, input, camera, UI, save, skill tree
    ├── world/              # World layout, platforms, floating damage text
    ├── rendering/          # Debug overlay renderer
    └── data_structures/    # HashTable, NTree, LinkedLists, Stack, Queue
```

---

## Building and Running

### Requirements

- **Windows** (the build scripts target Windows only)
- **GCC 7.3.0** (MinGW-w64, POSIX threads, SEH) — other versions may work but haven't been tested

Download GCC 7.3.0:

```
https://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win64/Personal%20Builds/mingw-builds/7.3.0/threads-posix/seh/x86_64-7.3.0-release-posix-seh-rt_v5-rev0.7z/download
```

After extracting, add `mingw64\bin` to your system `PATH`. For example:

```
C:\mingw730\mingw64\bin
```

You can verify it's on PATH by running `g++ --version` in a terminal — it should print `7.3.0`.

### Build & Run

Double-click `build_and_run.bat`, or from a terminal in the project root:

```bat
build_and_run.bat
```

The script compiles everything into `bin/main.exe` and launches the game automatically. If you only want to build without running:

```bat
build_and_run.bat --no-run
```

Or use PowerShell:

```powershell
.\build.ps1
```

### VS Code

Two tasks are configured in `.vscode/tasks.json`:

- **Build and prepare SFML game** — runs `build_and_run.bat --no-run` (default build task, `Ctrl+Shift+B`)
- **C/C++: g++.exe build active file** — direct `g++` compilation as a fallback

---

## Controls

Default bindings (all rebindable from the in-game menu, saved to `keybindings.cfg`):

| Action            | Default Key |
| ----------------- | ----------- |
| Move Left         | `A`         |
| Move Right        | `D`         |
| Jump              | `Space`     |
| Melee Attack      | `J`         |
| Shoot Projectile  | `K`         |
| Switch Projectile | `L`         |
| Special Attack    | `I`         |
| Use HP Potion     | `H`         |
| Open Skill Tree   | `T`         |

---

## Save Files

Save data is stored as plain text in `bin/saves/`. Two slots are available (`savegame1.txt`, `savegame2.txt`). Each save records player level, HP, damage, experience, and the full skill tree state. You can start a new game at any time without touching your saves.

---

## Notes

- SFML 2.5.1 is bundled in the repo (`SFML-2.5.1/`), so you don't need to install it separately
- The debug stdout flags in `src/core/Constants.h` let you trace individual data structure operations to the console — useful if you want to see the structures working at runtime
- The game window is fixed at 1200×800
