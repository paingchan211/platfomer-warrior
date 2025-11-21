# Polymorphism UML

```mermaid
classDiagram
direction TB

class Entity {
    <<abstract>>
    +{abstract} update(dt: float)
    +getBounds() FloatRect
}

class Character {
    <<abstract>>
    +takeDamage(dmg: int)
    +isAlive() bool
}

class Projectile {
    <<abstract>>
    +{abstract} reset()
    +{abstract} update(dt: float)
}

Entity <|-- Character
Entity <|-- Platform
Entity <|-- Projectile
Entity <|-- HPPotion

Character <|-- Player
Character <|-- Enemy
Enemy <|-- Boss

Projectile <|-- FireProjectile
Projectile <|-- IceProjectile
Projectile <|-- Meteor

note for Entity "
Base virtual hooks consumed by the engine
update(dt) runs per-frame logic
getBounds() feeds collision and hitscan checks"

note for Projectile "
Adds reset() contract for pooled bullets/meteors
Leaves update(dt) abstract so each projectile animates itself"

note for Character "
Keeps combat state virtual for specialization
Derived classes inject movement and AI in update(dt)"

class Platform {
    +update(dt: float) override
    +getBounds() FloatRect override
}

class HPPotion {
    +update(dt: float) override
    +getBounds() FloatRect override
}

class Player {
    +update(dt: float) override
    +getBounds() FloatRect override
    +handleInput(dt: float)
    +updatePhysics(dt: float, groundLevel: float, platforms: Platform**, numPlatforms: int)
}

class Enemy {
    +update(dt: float) override
    +getBounds() FloatRect override
    +update(dt: float, gravity: float, player: Player)
}

class Boss {
    +getBounds() FloatRect override
    +isAlive() bool override
    +update(dt: float, gravity: float, player: Player, isEnraged: bool)
}

class FireProjectile {
    +reset() override
    +update(dt: float) override
    +getBounds() FloatRect override
}

class IceProjectile {
    +reset() override
    +update(dt: float) override
    +getBounds() FloatRect override
}

class Meteor {
    +reset() override
    +update(dt: float) override
    +getBounds() FloatRect override
}
```
