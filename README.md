# 🕹️ Dōkutsu

**Dōkutsu** (洞窟) is a handcrafted 2D dungeon action game written in modern C++ with SDL2. Players explore a hostile, monster-infested world (soon-to-be cave system), wielding weapons and magic to survive and grow stronger.

<p align="center">
  <img src="https://github.com/user-attachments/assets/be55369a-0f7b-4e82-a008-a03845387389" alt="dying" height="300"/>
  <img src="https://github.com/user-attachments/assets/d266e37b-82f6-4054-a808-efa5017b41a8" alt="Screen Recording" height="300"/>
</p>

---

## 🎮 Gameplay Overview

- **Real-Time Combat**: Engage in dynamic melee combat using swappable weapons like swords, lances, and sais.
- **Enemy AI**: Fight against different enemies like spirits, raccoons, squids, and bamboo creatures—each with unique attack patterns and animations.
- **Magic System**: Cast powerful magic spells such as fireballs and healing spells with limited mana.
- **XP and Progression**: Gain experience points (EXP) and outlast increasingly aggressive enemies.
- **Camera & Rendering**: Smooth camera tracking and depth-sorted rendering for immersive visuals.

---

## 📦 Features

### ✅ Core Mechanics
- Directional movement and attack inputs
- Swappable weapons (`Q`) and magic (`W`)
- Magic and melee attack cooldowns
- Knockback system on successful hits

### ✅ Enemy System
- Intelligent behavior: enemies idle, chase, and attack based on proximity
- Frame-based animation and attack triggers
- Damage, knockback, invulnerability frames

### ✅ Visual Rendering
- Layered tilemap rendering (floor, grass, objects)
- Depth-based draw ordering of all visible sprites
- Real-time UI showing health, mana, and equipment

### ✅ Input Handling
- Keyboard movement (`Arrow Keys`)
- Attack with `Space` / `Tab`, cast magic with `E`

---

## 🔧 Tech Stack

| Component      | Description                        |
|----------------|------------------------------------|
| **Language**   | C++17                              |
| **Graphics**   | SDL2 + SDL_image                   |
| **UI**         | Custom HUD with health/mana bars   |
| **Entity System** | Polymorphic base sprite classes |
| **Tilemap**    | CSV-based layout loading           |
| **Animation**  | Directory-based frame parsing      |
| **Build**      | Simple Makefile or CMake (TBD)     |

---

## 🗺️ Map System

The game loads static tiles and enemies from CSV map layouts:
- `map/map_FloorBlocks.csv` – boundaries
- `map/map_Grass.csv` – decorative and collidable tiles
- `map/map_Objects.csv` – obstacles
- `map/map_Entities.csv` – players and enemies

---

## 📁 Project Structure

```
.
├── graphics/
│   ├── tilemap/
│   ├── monsters/
│   ├── weapons/
│   ├── player/
├── map/
│   ├── map_*.csv
├── src/
│   ├── main.cpp
│   ├── camera.h
│   ├── level.h
│   ├── player.h
│   ├── enemy.h
│   ├── weapon.h
│   ├── settings.h
```

---

## 🚀 Getting Started

### Requirements

- SDL2
- SDL2_image

### Compile & Run

```bash
g++ -std=c++17 -lSDL2 -lSDL2_image -o dokutsu main.cpp
./dokutsu
```

> Make sure to install SDL2 and SDL2_image via your OS package manager or build them locally.

---

## 🧠 Inspirations

Dōkutsu draws inspiration from classic dungeon crawlers and modern pixel-art indie games. Its architecture emphasizes modularity, OOP principles, and real-time rendering performance.

---


## 🙌 Credits

Made with ❤️ by Oscar Abreu
Assets using Tiled and PixelBoy and AAA's Assets.
https://pixel-boy.itch.io/ninja-adventure-asset-pack
