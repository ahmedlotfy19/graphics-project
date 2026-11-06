# ZOMBIE LAND: DMET 502 Project 1

A 2D side-view zombie lane-defense game in C++ and modern OpenGL 3.3.
Survivors defend their base against three waves of zombies coming across four lanes.

**Current state:** the whole game loop already runs: main menu → cutscene → gameplay → pause → win/lose, with sound.
All four parts are done: core & UI, players, zombies & waves, and the world & cutscene.
Each member only edits the files they own.

---

## ▶ Get the final version (start here for the video)

The finished game is on the **`main`** branch.

**You never cloned the repo:**
```
git clone https://github.com/ahmedlotfy19/graphics-project.git
cd graphics-project
```

**You already have it:** throw away any local test changes and pull the final version:
```
git checkout main
git pull
```
If `git pull` complains about local changes you don't need, run `git stash` first and then `git pull` again.

**Check you have the right version:** `git log --oneline -1` should show
`Member 4: night farmland lanes, Safe House base, props, fog, lane highlight and intro cutscene`,
or a newer commit.

**Run it:**
1. Open `Template.sln` in **Visual Studio 2022 or 2026**, which needs the "Desktop development with C++" workload.
2. Set the toolbar to **Release** and **x64**. Release runs smoother for recording. Debug also works.
3. Press **Ctrl+F5** to run without the debugger. The main menu with "ZOMBIE LAND" should appear.

Then follow the [video guide in §13](#13-submission--video).

---

## Table of contents
1. [First-time setup](#1-first-time-setup)
2. [Git workflow](#2-git-workflow)
3. [Project layout & ownership](#3-project-layout--ownership)
4. [How the game works (architecture)](#4-how-the-game-works-architecture)
5. [Coordinates, lanes and layout](#5-coordinates-lanes-and-layout)
6. [Drawing models: the Renderer2D API](#6-drawing-models-the-renderer2d-api)
7. [Other APIs: text, input, UI, audio](#7-other-apis-text-input-ui-audio)
8. [Task list per member](#8-task-list-per-member)
9. [Interfaces between members](#9-interfaces-between-members)
10. [Game rules quick reference](#10-game-rules-quick-reference)
11. [Sound](#11-sound)
12. [Before you push: checklist](#12-before-you-push-checklist)
13. [Submission & video](#13-submission--video)
14. [Troubleshooting](#14-troubleshooting)

---

## 1. First-time setup

1. Install **Visual Studio 2022** (or newer) with the **"Desktop development with C++"** workload.
2. Clone the repo:
   ```
   git clone https://github.com/ahmedlotfy19/graphics-project.git
   ```
3. Open `Template.sln`.
4. In the toolbar, set the configuration to **Debug** and the platform to **x64**. ⚠️ Win32 is *not* configured and will not build.
5. The project is set up for **Visual Studio 2026** (toolset v145). On Visual Studio 2022, VS asks to **retarget** the project: click **OK** (it switches to v143). If it doesn't ask, right-click the solution → **Retarget solution**.
6. Press **F5**. The main menu should appear.

Everything the game needs is already in the repo. Nothing else needs installing:

| Library | Where | Purpose |
|---|---|---|
| GLFW 3 | `External/include/GLFW`, `lib/glfw3.lib` | window, keyboard, mouse |
| GLAD | `include/glad`, `src/glad.c` | loads OpenGL 3.3 functions |
| GLM | `glm/` | vectors and matrices |
| miniaudio | `External/include/miniaudio.h` | music and sound effects |

The project uses **C++17**. All include and library paths are relative to the project folder, so the clone can live anywhere.

---

## 2. Git workflow

Everyone works on **their own branch** and merges into `main` when a piece is working.

```bash
git checkout main
git pull                          # get everyone's latest work
git checkout -b players           # once: your branch (players / enemies / world)

# ... work, build, test ...
git add -A
git commit -m "Melee player model + sword swing"
git push -u origin players        # first push; afterwards just: git push
```

Merging your work into `main`, for example when a feature is done:
```bash
git checkout main
git pull
git merge players                 # or open a Pull Request on GitHub
# build and run once more to be sure main still compiles
git push
git checkout players
git merge main                    # keep your branch up to date
```

Rules:
* **Pull `main` often**, at least daily, so conflicts stay small.
* **Never push code that doesn't compile to `main`.** A submission with compile errors gets a zero.
* Commit small and often, with messages that say what changed.

### Adding new files
Add new `.cpp`/`.h` files through VS (**right-click the folder in Solution Explorer → Add → New Item**) and save them in your own folder (`src/players/`, `src/enemies/` or `src/world/`).
That registers them in `Template.vcxproj`. If two people add files at the same time, git shows a conflict in `Template.vcxproj` and `.filters`. Resolve it by **keeping both sets of lines**.

---

## 3. Project layout & ownership

```
src/
├── main.cpp              Member 1  window, OpenGL context, main loop
├── Config.h              Member 1  ALL shared numbers (costs, HP, lane sizes, waves)
├── core/                 Member 1  engine
│   ├── Renderer2D.*                shape drawing, transforms, damage tint, grayscale
│   ├── Text.*                      built-in pixel font
│   ├── Input.*                     keyboard + mouse
│   ├── UI.*                        buttons, panels, health bars, coin icon
│   └── Audio.*, miniaudio_impl.c   music + sound effects
├── game/                 Member 1  game logic
│   ├── Game.*                      state machine, menus, HUD, win/lose
│   ├── World.h                     the struct holding all gameplay state
│   ├── Projectile.h                Projectile + Collectible structs
│   └── Collectibles.*              coins dropped by zombies
├── players/              Member 2
│   ├── Players.*                   the 3 player types
│   └── PlayerCards.*               the shop cards in the HUD
├── enemies/              Member 3
│   ├── Enemies.*                   the 3 zombie types
│   ├── Waves.*                     wave spawning
│   └── Combat.*                    collisions, attacks, damage
└── world/                Member 4
    ├── Lanes.*                     lane hover / select highlight
    ├── Environment.*               background, home base, props
    └── Cutscene.*                  intro cutscene
assets/audio/             sound files (anyone can replace them)
tools/                    make_placeholder_sounds.py
```

| Member | Area | Status |
|---|---|---|
| **1: Core & UI** | `main.cpp`, `Config.h`, `core/`, `game/` | ✅ Done |
| **2: Players** | `players/` | ✅ Done: Melee / Ranged / Defense models, jump and duck, cards |
| **3: Enemies & combat** | `enemies/` | ✅ Done: Spitter / Bone Roller / Brute models, High/Low attacks, dodging, waves |
| **4: World & cutscene** | `world/` | ✅ Done: night farmland lanes, Safe House base, 7 simple props (car, dead trees, graves, sandbags, fence), 17 s cutscene |

Each file of Members 2, 3 and 4 starts with a comment block listing what it covers.

### Ownership rules
* **Only edit files you own.** If you need a change somewhere else, ask the owner.
* In `Players.h` and `Enemies.h`, fields under `// ---- REQUIRED ----` are read by other members' code. **Don't rename or remove them.** Add your own fields under `// ---- Member N's own fields ----`.
* `Config.h` is shared. You can **tune numbers** in it (speeds, costs, sizes). Tell the group before **renaming or removing** anything.
* `World.h`, `Projectile.h`: if you need a new field in `Projectile` or `Collectible`, ask Member 1 or tell the group first.

---

## 4. How the game works (architecture)

### The main loop
`main.cpp` opens a 1280×720 window and, every frame, calls `Game::frame(dt)`. `dt` is the frame time in seconds, capped at 0.05.
`Game` is a **state machine**:

```
           ┌────────────── START ──────────────┐
MainMenu ──┤                                   ▼
   ▲  │    └─ SOUND SETTINGS ─► SoundSettings  Cutscene ── finishes / SPACE ──► Playing
   │  └─ EXIT (closes window)                                              │  ▲   │
   │                                                                  ESC  ▼  │ESC│ zombie reaches base / all waves cleared
   └──────────────── MAIN MENU ◄──────────── Paused ◄──────────────────────┘      ▼
   └──────────────── MAIN MENU ◄──────────── Won / Lost  (RESTART → Playing)
```

### One frame of gameplay (`Game::updatePlaying`, then `Game::drawScene`)
**Update order:**
```
PlayerCards::update   (Member 2)  card hover/click, buying, placing
Lanes::update         (Member 4)  lane hover + click toggle
Players::update       (Member 2)  movement, jump/duck, firing
Waves::update         (Member 3)  spawning
Enemies::update       (Member 3)  movement, attacks
Combat::update        (Member 3)  collisions, projectile hits, damage
Collectibles::update  (Member 1)  coin physics, pick-up
remove dead entities  (Member 1)  everything with alive == false is erased
check win / lose      (Member 1)
```
Cards update before lanes on purpose. When a card is clicked it calls `Input::consumeClick()`, so the lane under the cursor doesn't toggle as well.

**How attacks work:** `Players.cpp` and `Enemies.cpp` only decide **when** an attack happens. `Combat.cpp` decides **who gets hit**.

| Attack | Started by | What Combat does |
|---|---|---|
| Melee axe | `Players::update` sets `p.strikeNow` every 1 s | hits the nearest zombie touching that player |
| Brute club | `Enemies::update` sets `e.strikeNow` when the club comes down | hits the nearest player touching the Brute for 2 |
| Crossbow bolt | `Players::update` adds a projectile every 1.7 s | moves it and hits the first zombie it reaches |
| Spit / skull | `Enemies::update` adds a projectile | moves it and hits the front player, unless they duck (spit) or jump (skull) |

Every hit goes through `Players::damage` or `Enemies::damage`, which handle sound, death, coin drops and the Defense rule.

**Draw order** (back to front):
```
Environment::drawBackground → Lanes::draw → Environment::drawHomeBase → coins
→ for each lane, top to bottom: Players::draw, Enemies::draw
→ projectiles (Players::drawProjectile / Enemies::drawProjectile)
→ Environment::drawForeground → health bars (automatic) → HUD
```

### The World struct
**All gameplay state lives in one struct**, `World` (`src/game/World.h`). It is passed by reference to every system:

```cpp
struct World {
    std::vector<Player>      players;      // Member 2's struct (Players.h)
    std::vector<Enemy>       enemies;      // Member 3's struct (Enemies.h)
    std::vector<Projectile>  projectiles;  // shared (Projectile.h)
    std::vector<Collectible> collectibles; // Member 1
    int   coins;                           // starts at START_COINS
    float time;                            // seconds of gameplay, pauses with the game
    LaneState lanes;                       // Member 4: hovered, selected[]
    CardState cards;                       // Member 2: hovered, selected
    WaveState waves;                       // Member 3: index, spawned, timer...
};
```
Each member owns the *type* of their part (`Player`, `Enemy`, `LaneState`, `CardState`, `WaveState`), defined in their own header. You can add fields there without touching `World.h`.

**To remove anything** (a dead player or zombie, a projectile that hit, a collected coin), set `alive = false`. Core erases it at the end of the frame.
**Don't** erase from the vectors yourself while looping over them.

---

## 5. Coordinates, lanes and layout

* The screen is a **virtual 1280×720** canvas. **Origin (0,0) is the bottom-left corner; x goes right, y goes UP.**
  The renderer scales and letterboxes this to any window size, and the mouse is converted to the same coordinates.
* There are **4 lanes** (`NUM_LANES`), each `LANE_HEIGHT = 145` px tall. **Lane 0 is the top lane** and lane 3 is the bottom one.

```
y=720 ┌──────────────────────────────────────────────────────────┐
      │ HUD: coins │ player cards │ hints          │ wave info  │
y=600 ├────────┬─────────────────────────────────────────────────┤ FIELD_TOP
      │        │ lane 0                                          │
y=455 │  HOME  ├─────────────────────────────────────────────────┤
      │  BASE  │ lane 1                                          │
y=310 │ x 0..  ├─────────────────────────────────────────────────┤
      │  150   │ lane 2                                          │
y=165 │        ├─────────────────────────────────────────────────┤
      │        │ lane 3                                          │
y=20  └────────┴─────────────────────────────────────────────────┘ FIELD_BOTTOM
      x=0    x=150                                            x=1280
             players spawn at x=190 ──►        ◄── zombies spawn at x=1250
```

Helpers in `Config.h`. Use them instead of hard-coding numbers:

| Function / constant | Meaning |
|---|---|
| `laneBottomY(l)`, `laneTopY(l)` | bottom / top edge of lane `l` |
| `laneCenterY(l)` | vertical centre of lane `l` |
| `laneGroundY(l)` | **where feet stand**, 22 px above the lane bottom. Draw characters from here |
| `laneAtY(y)` | lane under a y coordinate, or -1 |
| `HOME_BASE_RIGHT` (150) | right edge of the base. **A zombie touching it = game over** |
| `PLAYER_SPAWN_X` (190) / `ENEMY_SPAWN_X` (1250) | spawn positions |
| `FIELD_TOP` (600) | everything above is HUD |

**Character size budget:** a lane is 145 px tall. Keep models around **80–100 px tall** from `laneGroundY`. The health bar is drawn at `height + 8` above the ground, so set your entity's `height` field to the model's real height.

---

## 6. Drawing models: the Renderer2D API

Every model is built from shapes. There are no textures. All drawing functions take a `Renderer2D& r`.

### Example: a complete small model
```cpp
void drawSurvivor(Renderer2D& r, float x, float groundY, float anim, float damage)
{
    r.push();                              // save transform + filters
    r.translate(x, groundY);               // local origin = between the feet
    r.setDamageTint(damage);               // 0 = normal, 1 = fully red

    // legs (walk cycle)
    float swing = std::sin(anim * 8.0f) * 0.3f;
    r.push(); r.translate(-6, 34); r.rotate( swing); r.rect(-4, -34, 8, 34, Colors::rgb(50, 60, 90)); r.pop();
    r.push(); r.translate( 6, 34); r.rotate(-swing); r.rect(-4, -34, 8, 34, Colors::rgb(45, 55, 85)); r.pop();

    // torso + belt
    r.roundedRect(-14, 32, 28, 36, 6, Colors::rgb(120, 90, 60));
    r.rect(-14, 34, 28, 5, Colors::rgb(60, 40, 25));

    // head + hair + eye
    r.circle(0, 80, 12, Colors::rgb(235, 195, 160));
    r.pie(0, 82, 13, 0.2f, 2.9f, Colors::rgb(70, 45, 30));
    r.circle(5, 80, 2, Colors::rgb(0, 0, 0));

    r.pop();                               // restore
}
```

### Shapes
`(x, y)` is the **bottom-left** corner unless stated otherwise. Angles are in **radians**, counter-clockwise from +x.

| Function | Draws |
|---|---|
| `rect(x, y, w, h, c)` | rectangle |
| `rectGradient(x, y, w, h, bottom, top)` | vertical gradient (sky, ground) |
| `rectOutline(x, y, w, h, thickness, c)` | hollow rectangle |
| `roundedRect(x, y, w, h, radius, c)` | rounded rectangle (bodies, cards) |
| `triangle(a, b, c, col)` | triangle, points are `glm::vec2` |
| `circle(cx, cy, r, c)` | circle |
| `ellipse(cx, cy, rx, ry, c)` | ellipse (shadows, heads) |
| `ring(cx, cy, innerR, outerR, c)` | hollow circle |
| `pie(cx, cy, r, a0, a1, c)` | filled slice (hair, helmets, mouths) |
| `arc(cx, cy, r, thickness, a0, a1, c)` | curved thick line (bows, swing trails) |
| `line(a, b, thickness, c)` | thick line (swords, sticks, cracks) |
| `polygon({p0, p1, ...}, c)` | **convex** polygon |
| `fullscreen(c)` | covers the whole screen (fades, overlays) |

The optional last parameter `segments` on circles and arcs controls smoothness (default 32/24).

### Transforms (stack)
| Function | Effect |
|---|---|
| `push()` / `pop()` | save / restore the transform **and** the filters. Always pair them |
| `translate(x, y)` | move the origin |
| `rotate(rad)` | rotate around the current origin |
| `scale(sx, sy)` / `scale(s)` | scale. **`scale(-1, 1)` mirrors** a model so it faces left (useful for zombies) |

Transforms combine: translate to a joint, then rotate, then draw the limb.

### Filters (also saved by push/pop)
| Function | Use |
|---|---|
| `setDamageTint(amount)` | **Spec requirement.** Blends everything toward red. Use `1.0f - (float)hp / maxHp` |
| `setGrayscale(true)` | **Spec requirement.** Use for locked player cards |
| `setAlpha(a)` | fading in and out (cutscene, death animation) |

### Colours
`Color` is a `glm::vec4` (r, g, b, a from 0 to 1). The easiest way to make one is `Colors::rgb(255, 128, 0)` or `Colors::rgb(255, 128, 0, 120)` with alpha.
Ready-made: `Colors::White`.

### Tips
* **Painter's algorithm:** shapes draw in the order you call them. Draw back parts first (far arm, far leg), then the body, then front parts.
* Use a per-entity `anim` timer (seconds) with `std::sin(anim * speed)` for bobbing, walking, breathing and swinging.
* Don't call OpenGL directly. Everything goes through `Renderer2D`, which batches shapes for speed.
* ⚠️ **Spec:** no scene component may be a single primitive, and types must differ in *shape and equipment*, **not only colour**. Weak modelling detail is a significant deduction.

---

## 7. Other APIs: text, input, UI, audio

### Text (`core/Text.h`)
```cpp
Text::draw(r, "WAVE 1", x, y, size, color);                     // left-aligned, (x,y) = bottom-left
Text::draw(r, "HELLO", x, y, 20, Colors::White, Align::Center); // Align::Left / Center / Right
Text::drawShadowed(r, "SUBTITLE", x, y, 18, Colors::White, Align::Center);  // readable on busy backgrounds```
`size` is the letter height in pixels. Lowercase is drawn as capitals, and `\n` starts a new line. This is ideal for cutscene subtitles.

### Input (`core/Input.h`)
```cpp
Input::keyPressed(GLFW_KEY_W)      // true only on the frame the key went down
Input::keyDown(GLFW_KEY_W)         // true while held
Input::mousePressed(GLFW_MOUSE_BUTTON_LEFT)
Input::mouse()                     // glm::vec2 in VIRTUAL coordinates (y up)
Input::mouseInRect(x, y, w, h)
Input::consumeClick()              // after this, mousePressed(LEFT) is false for the rest of the frame
```
Spec keys: **W / Up = jump**, **S / Down = duck**, **Esc = pause** (Esc is already handled by Core).

### UI (`core/UI.h`)
```cpp
if (UI::button(r, "OK", x, y, w, h)) { ... }   // draws, handles hover/click, plays the click sound
UI::panel(r, x, y, w, h);                       // dark framed box
UI::healthBar(r, cx, y, hp, maxHp);             // already drawn for every player/zombie automatically
UI::coinIcon(r, cx, cy, radius, spin);          // the coin graphic
```

### Audio (`core/Audio.h`)
```cpp
Audio::play(Sfx::PlayerAttack);
```
See [§11](#11-sound) for the list.

### Gameplay helpers other members provide
```cpp
Players::spawn(w, PlayerType::Ranged, lane);
Players::damage(w, player, amount);      // ALWAYS use this to hurt a player (sound, death and the Defense rule handled)
Enemies::spawn(w, EnemyType::Melee, lane);
Enemies::damage(w, enemy, amount);       // ALWAYS use this to hurt a zombie (sound + death + coin drop)
Collectibles::spawn(w, lane, x, COIN_DROP_VALUE);   // already called by Enemies::damage on death
PlayerCards::cost(type), PlayerCards::isUnlocked(w, type)
Waves::currentWave(w), Waves::allSpawned(w)
```

---

## 8. Task list per member

Each item is a spec requirement.

### Member 2: Players (`src/players/`)

**Models (`Players::draw`)**
- [x] **Melee** player: distinct body, holds a close-range weapon (bat, machete, axe...) that **animates in the hand** when attacking
- [x] **Ranged** player: distinct body, holds a ranged weapon (crossbow, slingshot, rifle...)
- [x] **Defense** player: distinct, bulky and stationary (riot shield, barricade carrier, armoured...)
- [x] The three types differ in **shape, equipment and silhouette, not colour**
- [x] Walk animation, attack animation, jump pose, duck pose
- [x] Red damage tint: `r.setDamageTint(1.0f - (float)p.hp / p.maxHp)` (or scale the model down per hit; pick one)
- [x] Set `p.height` to the model's real height so the health bar sits right above it

**Behaviour (`Players::update`)**
- [x] Melee and Ranged walk right from `PLAYER_SPAWN_X` automatically. **Defense never moves** and is placed where you click on the field
- [x] Stop while touching a zombie (`p.engaged`), resume when it dies
- [x] Attack automatically from the moment they're placed, **even with no enemy in the lane**
- [x] Ranged: fire projectiles into `w.projectiles` (`fromPlayer = true`, `height = AttackHeight::Mid`). The projectile **animates** and **disappears on hit**. It never harms players
- [x] **Jump** on W/Up and **duck** on S/Down for every player in a **selected** lane (`w.lanes.selected[p.lane]`). Set `p.pose` (Jumping/Ducking) and `p.y` (jump height). Defense players ignore this
- [x] All players deal `PLAYER_DAMAGE` (1) per attack
- [x] At the right edge they leave the field and are removed
- [x] Defense rule: `Players::damage` passes the hit to a living Defense player standing **to the right of** (in front of) the target in the same lane

**Cards (`PlayerCards.cpp`)**
- [x] Draw a **mini version of each model** on its card, with the name and cost
- [x] Locked (`coins < cost`): grayscale (`r.setGrayscale(true)`), **no hover and no click**
- [x] Unlocked: normal colours, hover effect, clickable
- [x] Unlock and re-lock happen automatically (already true: `isUnlocked` checks coins every frame)
- [x] Placement flow (see [§9](#9-interfaces-between-members)). Deduct `cost(type)` from `w.coins` and play `Sfx::Place`

Costs are already set to spec in `Config.h`: Melee 10, Ranged 20 (2×), Defense 30 (3×), start coins 30 (3 melee).

### Member 3: Enemies & combat (`src/enemies/`)

**Models (`Enemies::draw`)**
- [x] **Easy: Spitter**, a thin hunched zombie whose acid sac swells before it spits. The spit flies at head height (`AttackHeight::High`, so **duck**)
- [x] **Medium: Bone Roller**, a stocky zombie in overalls with a sack of skulls. It bowls skulls along the ground (`AttackHeight::Low`, so **jump**)
- [x] **Hard: Brute**, armoured with a spiked shoulder pad, helmet, glowing eyes and a nail club. It runs and hits for 2
- [x] Walk/run cycles, attack animations (swelling acid sac, skull throw, club swing) and a red damage tint
- [x] Projectile visuals: wobbling acid spit and a rolling skull
- [x] `Enemies::drawModel(r, type, x, groundY, anim, ...)` draws any zombie without a World (for the cutscene)

**Behaviour (`Enemies.cpp`, `Combat.cpp`)**
- [x] Move left (Spitter 30 px/s, Roller 26 px/s, Brute 72 px/s), and stop while touching a player
- [x] Attack from the moment they appear, even with no player in the lane (spit every 5 s with 300 px range, skull every 3.6 s with 520 px range)
- [x] Dodging: Ducking avoids High, Jumping avoids Low. Defense players never dodge and absorb hits
- [x] Damage 1, Brute 2. Melee players set `strikeNow` once per second when the axe comes down, and Combat hits the zombie they touch
- [x] Combat moves all projectiles and checks every hit: axe, club, bolts, spit and skulls
- [x] On death: removed, sound played, coin dropped

**Waves (`Waves.cpp`)**
- [x] Wave 1: 3 easy. Wave 2: 5 (3 easy + 2 medium). Wave 3: 10 (4 easy + 3 medium + 3 hard). Shuffled, random lanes, never the same lane twice in a row
- [x] The next wave starts 4 s after the field is clear. Spawn gaps shrink each wave
- [ ] Balance pass after playtesting with the real environment (speeds and intervals are at the top of `Enemies.cpp`)

### Member 4: World & cutscene (`src/world/`)

**Environment (`Environment.cpp`)**
- [x] Themed background: night sky, ruined city or fields, whatever fits the story
- [x] Lane ground that reads clearly as 4 separate lanes
- [x] **Home base** at `x ∈ [0, 150]`: detailed (fortified house, bunker, barricaded shop...) and **clearly visible against the environment**, not just a recolour
- [x] **At least 5 environment props** that fit the theme and sit naturally in the scene: wrecked cars, barricades, streetlights, sandbags, barrels, fences, dead trees, graves...
- [x] `drawForeground` for anything that should be drawn over characters (fog, rain, flickering light)
- [x] Animate a little using the `time` parameter: the Safe House windows flicker and fog drifts over the lanes

**Lanes (`Lanes.cpp`)**: the logic already follows the spec. Polish the look:
- [x] Hover highlights the **entire lane**
- [x] Click toggles selected/deselected, multiple lanes at once (done). Make "selected" look clearly different from "hovered"

**Cutscene (`Cutscene.cpp`)**
- [x] **10–20 s** long (`DURATION`, 17 s)
- [x] Shows **at least one player and one zombie, animated and interacting**
- [x] Establishes **what is being defended and why** (the home base and who is inside)
- [x] A short **dialogue** between the survivors and the zombie(s), with subtitles via `Text::drawShadowed`. It **must not be text only**
- [x] Logically leads into the gameplay. The theme, characters and base should match what the player sees next
- [x] Reuse Member 2's and 3's model drawing functions so the characters look identical (see [§9](#9-interfaces-between-members))
- [x] Use `setAlpha` and `fullscreen` for fades and camera-like transitions

### Member 1: Core & UI (done; keeps maintaining)
- [x] Window, OpenGL 3.3 core, main loop, renderer, text, input
- [x] Main menu (Start → cutscene → game, Sound settings, Exit), Pause (Esc: Continue / Restart / Main menu), Win/Lose (Restart / Main menu)
- [x] Coin counter at the top, collectibles, health bars for everyone, win/lose detection, freeze on loss
- [x] Sound bonus: music, SFX, sound settings menu (volume up/down, mute)
- [ ] Integrate everyone's work, final testing, final README and zip

---

## 9. Interfaces between members

Agree on these in the group chat early. Most are already wired up.

### Member 2 ↔ Member 3: combat
| Who | Does what |
|---|---|
| Member 2 | sets `Player::pose` (`Normal` / `Jumping` / `Ducking`) and `Player::y` |
| Member 2 | implements `Players::damage` (redirects the hit to a Defense player in front) and sets `Player::strikeNow` when the axe lands |
| Member 2 | spawns player projectiles into `w.projectiles` with `fromPlayer = true` |
| Member 3 | `Combat::update` moves **all** projectiles, checks hits and calls `Players::damage` / `Enemies::damage` |
| Member 3 | dodge rule: `High` misses a `Ducking` player, `Low` misses a `Jumping` player, `Mid` always hits |

### Member 2 ↔ Member 4: placement & lane selection
Current flow (spec section 3.2): **click lane(s) to select them, then click an unlocked card**. Melee and Ranged: one player is placed at the left end of **each selected lane** you can afford. Defense: after clicking its card, a see-through guard follows the mouse. **Click a spot on the field** to place one there in each selected lane, or click the card again to cancel.
Jump/duck applies to players in `w.lanes.selected[]`. If you change the flow, keep two things: lane selection must stay toggleable, and cards must call `Input::consumeClick()`.

### Member 4 ↔ Members 2 & 3: the cutscene uses the real models
Members 2 and 3: expose a drawing function that doesn't need a `World`, for example:
```cpp
// Players.h
void drawModel(Renderer2D& r, PlayerType t, float x, float groundY, float anim, Pose pose = Pose::Normal);
// Enemies.h
void drawModel(Renderer2D& r, EnemyType t, float x, float groundY, float anim);
```
Your `draw()` can call it, and Member 4 calls it from the cutscene.

### Everyone ↔ Member 1
* Need a new sound? Add a name in `Audio.h` (or ask), and drop the `.wav` into `assets/audio/`.
* Need a new shape or renderer feature? Ask Member 1.
* Need a new field in `Projectile`/`Collectible`/`World`? Ask Member 1.

---

## 10. Game rules quick reference

| | Players | | | Zombies | | |
|---|---|---|---|---|---|---|
| **Type** | Melee | Ranged | Defense | Easy (upper-ranged) | Medium (lower-ranged) | Hard (melee) |
| **HP** | 5 | 5 | 7 | 5 | 5 | 7 |
| **Damage** | 1 | 1 | none (blocks) | 1 | 1 | **2** |
| **Moves** | right | right | **never** | left, normal | left, normal | left, **runs** |
| **Attack** | close | projectile | none | from **above** (duck) | from **below** (jump) | close |
| **Cost / unlock** | 10 | 20 | 30 | | | |

* Start with **30 coins**. Each zombie drops a coin worth **20** (`COIN_DROP_VALUE`) that bounces onto the lane and is collected when a player touches it.
* A card is unlocked while `coins >= cost` and **re-locks** if coins drop below.
* **Win:** all 3 waves spawned and every zombie dead. **Lose:** any zombie reaches the home base, and then everything freezes.
* Waves: 3 → 5 → 10 zombies.
* Everyone's health must be visible at all times. Core already draws a segmented health bar above each character.

All of these numbers live in `Config.h`.

---

## 11. Sound

Files: `assets/audio/<name>.wav` (`.mp3` and `.flac` also work). Names are defined in `src/core/Audio.h`:

| Name | When | Played by |
|---|---|---|
| `music_menu` | menus + cutscene (loops) | Core |
| `music_game` | gameplay (loops) | Core |
| `click` | any UI button | Core |
| `lane_select` | lane toggled | Member 4 (already) |
| `place` | player placed | Member 2 (already) |
| `player_attack` | player swings / fires | **Member 2: add** |
| `enemy_attack` | zombie spits / bowls / swings | Member 3 (done) |
| `hit` | someone takes damage | Players/Enemies `damage()` (already) |
| `player_death`, `enemy_death` | deaths | `damage()` (already) |
| `coin` | coin collected | Core |
| `jump` | jump | **Member 2: add** |
| `win`, `lose` | end of game | Core |

The current files are synthesized placeholders. To regenerate them, run `python tools/make_placeholder_sounds.py` from the project folder.
**Replace any of them** with a better (royalty-free) sound under the same name. A missing file only prints a warning and never breaks the game.
The Sound Settings menu (volume ±10%, mute) affects everything.

---

## 12. Before you push: checklist

- [ ] It **builds** in **Debug | x64** with no errors
- [ ] You ran it: menu → start → play a bit → pause → restart works
- [ ] You only changed files you own (check `git status`)
- [ ] New files were added through VS, so they're in `Template.vcxproj`
- [ ] No debug spam left in the console

---

## 13. Submission & video

**Zip (Member 1):** `[Team name]-[Member name].zip` containing the source and project files (`src/`, `include/`, `External/`, `glm/`, `lib/`, `assets/`, `Template.sln`, `Template.vcxproj*`).
Before zipping, test it from a **fresh clone** on another machine if possible.

### Video rules (from the spec)
* **2 to 5 minutes** long, with a **voiceover** explaining what the game is, its aim, and how to play.
* It must follow this **exact order**:
  1. University name and logo
  2. Course name and semester
  3. Project name
  4. Students' names
  5. The rest of the video (gameplay)
  6. Thank you
* It may be published online, so make it clean: no desktop clutter, no notifications, no long silences.
* Submit it as a **separate link** in the same form as the zip, and give the staff access to it.

### Recording setup
* **Recorder:** OBS Studio (free) gives the best quality. The Xbox Game Bar built into Windows also works: press **Win + G**, then the record button, or **Win + Alt + R** to start and stop.
* **Window:** maximise the game window. It scales to any size and keeps the 16:9 picture. Record at 1920×1080 or 1280×720.
* **Sound:** record the game audio. The music and sound effects are part of the sound bonus, so the graders should hear them. Lower the game volume in Sound Settings if it covers the voiceover.
* **Build:** use **Release | x64** for smooth frames.
* Turn on Windows **Focus assist / Do not disturb** so no pop-ups appear.
* **Recommended:** record the gameplay first without talking, then record the voiceover over it in an editor (CapCut, Clipchamp, DaVinci Resolve, Shotcut). It's much easier than talking while playing.

### Shot list (about 4 minutes)

| # | Time | Show on screen | What to do in the game |
|---|---|---|---|
| 1 | 0:00–0:05 | **GUC name and logo** | Title card made in the editor |
| 2 | 0:05–0:10 | **DMET 502 Computer Graphics, Winter 2026** | Title card |
| 3 | 0:10–0:15 | **ZOMBIE LAND** | Title card, or the game's main menu |
| 4 | 0:15–0:25 | **Students' names** | Title card |
| 5 | 0:25–0:40 | Main menu | Hover the buttons, open **SOUND SETTINGS**, press **+ VOL** and **- VOL**, toggle **MUTE**, then **BACK** |
| 6 | 0:40–0:58 | **Cutscene (do NOT skip it)** | Click **START GAME** and let the 17 s intro play to the end |
| 7 | 0:58–1:20 | Wave 1, how to play | Hover a lane (it lights up), click 2 lanes to **select** them (yellow outline and arrow), then click the **MELEE** card. A melee player appears in each selected lane. Show that **RANGED** and **DEFENSE** are **grey and locked** |
| 8 | 1:20–1:45 | Wave 1, fighting | The Spitter spits at head height. Select its lane and hold **S / Down** to **duck** under it. Kill zombies, show a coin **dropping, bouncing and being collected**, and show the coin counter going up |
| 9 | 1:45–2:10 | Unlocking cards | Once coins ≥ 20, **RANGED** turns colourful. Place one and show its crossbow bolts. At ≥ 30, select a lane, click **DEFENSE**, then **click a spot on the field** to place the guard there. Show that it doesn't move and **protects the players behind it** |
| 10 | 2:10–2:40 | Wave 2 | **Bone Rollers** roll skulls along the ground. Select the lane and press **W / Up** to **jump** over them |
| 11 | 2:40–3:10 | Wave 3 | The armoured **Brute** runs fast and hits for 2. Show the **red damage tint** on hurt characters and the health bars |
| 12 | 3:10–3:20 | Pause menu | Press **Esc**, show **CONTINUE / RESTART / MAIN MENU**, press CONTINUE |
| 13 | 3:20–3:30 | **Win** | Clear wave 3 and show the win screen with **RESTART / MAIN MENU** |
| 14 | 3:30–3:50 | **Lose** | Press RESTART, skip the cutscene with **SPACE**, place **nobody**, and wait about 40 s until a zombie reaches the Safe House. Speed this part up in the editor. Show the lose screen, where everything freezes |
| 15 | 3:50–3:58 | Main menu → EXIT | Click **MAIN MENU**, then **EXIT** to show the game closes |
| 16 | 3:58–4:05 | **Thank you** | Title card |

**Tips for a clean run**
* **Easy win:** in wave 1, put a melee player in every lane that has a zombie. Spend new coins on Ranged players. Put a Defense player in front once you have 30.
* **Lane selection:** you can select several lanes at once, so one card click places a player in **each** selected lane, as long as you have enough coins.
* **Jump and duck:** these only work for players in **selected** lanes. Defense players never jump or duck.
* **Re-takes:** use Pause → RESTART to try a part again. Skip the cutscene with **SPACE** after you've recorded it once.

### Voiceover script (edit freely)

> **Intro (over the title cards):** "This is Zombie Land, our project for DMET 502 Computer Graphics at the German University in Cairo, Winter 2026."
>
> **Menu:** "The game starts at the main menu. From here you can start the game, open the sound settings to raise, lower or mute the volume, or exit."
>
> **Cutscene:** "Starting the game plays an animated cutscene. The last safe house in town shelters twelve survivors and the town's last food and medicine. A Spitter zombie attacks, the survivor ducks its acid and fights back, but a Brute warns that thousands more are coming, so the survivors run to the lanes to defend the house."
>
> **Aim:** "The aim is to protect the Safe House on the left. Zombies come from the right in three waves. If a single zombie reaches the house, you lose. Survive all three waves to win."
>
> **How to play:** "Click a lane to select it. You can select several. Then click a card to place a survivor at the start of each selected lane. Melee survivors fight up close with an axe, Ranged survivors shoot crossbow bolts, and Defense survivors are placed wherever you click on the lane, hold that spot and protect everyone behind them. Every survivor except the defender walks forward and attacks automatically."
>
> **Coins:** "Placing survivors costs coins. Every zombie you kill drops a coin that bounces onto the lane, and walking into it collects it. Cards you can't afford stay grey and locked, and unlock as soon as you have enough coins."
>
> **Zombies:** "There are three zombies. The Spitter spits acid at head height, so you press S or Down to duck. The Bone Roller bowls skulls along the ground, so you press W or Up to jump. The armoured Brute runs fast and hits twice as hard. Jumping and ducking apply to every survivor in the selected lanes."
>
> **Menus:** "Escape pauses the game, where you can continue, restart or return to the main menu. At the end, the win or lose screen lets you restart or go back to the menu."
>
> **Outro:** "Thank you for watching."

### Before you upload
- [ ] Length is between 2 and 5 minutes
- [ ] The order is logo → course → project name → names → gameplay → thank you
- [ ] The video shows the cutscene, all 3 survivors, all 3 zombies, jump, duck, locked and unlocked cards, coins, all 3 waves, pause, win, lose, sound settings and exit
- [ ] The voiceover is clear and the game sound is audible
- [ ] The drive link is shared so that **anyone with the link can view**

---

## 14. Troubleshooting

| Problem | Fix |
|---|---|
| Build errors about `glfw3.h` / `glad.h` not found, or linker errors about `glfw...` | Platform must be **x64**, not Win32 |
| "OpenGL header already included" | Include `core/Renderer2D.h` (which includes glad) **before** anything that includes GLFW |
| No sound | Check the console for `[Audio] missing sound`. The working directory must be the project folder (the VS default) |
| A new .cpp "isn't compiled" / unresolved external | Add it via VS **Add → Existing Item** so it's in the vcxproj |
| Merge conflict in `Template.vcxproj` | Keep both people's `<ClCompile>`/`<ClInclude>` lines |
| Something draws in the wrong place | y goes **up**, origin bottom-left. Check `push`/`pop` are balanced |
| Model doesn't turn red when hit | Call `setDamageTint` **after** `push()` and **before** the shapes |
