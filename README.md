# LAST STAND: DMET 502 Project 1

A 2D side-view zombie lane-defense game in C++ and modern OpenGL 3.3.
Survivors defend their base against three waves of zombies coming across four lanes.

**Current state:** the whole game loop already runs: main menu → cutscene → gameplay → pause → win/lose, with sound.
Member 1's systems are finished. Everything owned by Members 2, 3 and 4 is a **working placeholder** (coloured boxes, basic logic) in that member's own files.
Your job is to replace your placeholders with the real thing. You should never need to edit another member's files.

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
   git clone <repo-url>
   ```
3. Open `Template.sln`.
4. In the toolbar, set the configuration to **Debug** and the platform to **x64**. ⚠️ Win32 is *not* configured and will not build.
5. If VS asks to retarget the project (Platform Toolset / Windows SDK), click **OK**.
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
archive/                  old triangle demo (not compiled)
```

| Member | Area | Status |
|---|---|---|
| **1: Core & UI** | `main.cpp`, `Config.h`, `core/`, `game/` | ✅ Done |
| **2: Players** | `players/` | 🟨 Placeholder: boxes, walking, card lock/unlock |
| **3: Enemies & combat** | `enemies/` | 🟨 Placeholder: boxes, walking, basic waves, touching = trade hits |
| **4: World & cutscene** | `world/` | 🟨 Placeholder: flat lanes, box base, 4 s placeholder cutscene. Lane hover/select works |

Every placeholder file starts with a comment block saying what the owner must add. Inline `// TODO Member N` comments mark the exact spots.

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
    r.circle(5, 80, 2, Colors::Black);

    r.pop();                               // restore
}
```

### Shapes
`(x, y)` is the **bottom-left** corner unless stated otherwise. Angles are in **radians**, counter-clockwise from +x.

| Function | Draws |
|---|---|
| `rect(x, y, w, h, c)` | rectangle |
| `rectCentered(cx, cy, w, h, c)` | rectangle centred on a point |
| `rectGradient(x, y, w, h, bottom, top)` | vertical gradient (sky, ground) |
| `rectHGradient(x, y, w, h, left, right)` | horizontal gradient |
| `rectOutline(x, y, w, h, thickness, c)` | hollow rectangle |
| `roundedRect(x, y, w, h, radius, c)` | rounded rectangle (bodies, cards) |
| `triangle(a, b, c, col)` | triangle, points are `glm::vec2` |
| `quad(a, b, c, d, col)` | any 4-point shape, points in order around the edge |
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
| `rotate(rad)` / `rotateDeg(deg)` | rotate around the current origin |
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
Ready-made: `Colors::White, Black, Red, Green, Yellow, Gray, Clear`. Helpers: `Colors::lerp(a, b, t)`, `Colors::withAlpha(c, a)`.

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
Text::drawShadowed(r, "SUBTITLE", x, y, 18, Colors::White, Align::Center);  // readable on busy backgrounds
float w = Text::width("HELLO", 20);
```
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
Players::damage(w, player, amount);      // ALWAYS use this to hurt a player (sound + death handled)
Players::isProtected(w, player);         // defense rule
Enemies::spawn(w, EnemyType::Melee, lane);
Enemies::damage(w, enemy, amount);       // ALWAYS use this to hurt a zombie (sound + death + coin drop)
Collectibles::spawn(w, lane, x, COIN_DROP_VALUE);   // already called by Enemies::damage on death
PlayerCards::cost(type), PlayerCards::isUnlocked(w, type)
Waves::currentWave(w), Waves::allSpawned(w)
```

---

## 8. Task list per member

Each item is a spec requirement. Tick them off in your placeholder file's header.

### Member 2: Players (`src/players/`)

**Models (`Players::draw`)**
- [ ] **Melee** player: distinct body, holds a close-range weapon (bat, machete, axe...) that **animates in the hand** when attacking
- [ ] **Ranged** player: distinct body, holds a ranged weapon (crossbow, slingshot, rifle...)
- [ ] **Defense** player: distinct, bulky and stationary (riot shield, barricade carrier, armoured...)
- [ ] The three types differ in **shape, equipment and silhouette, not colour**
- [ ] Walk animation, attack animation, jump pose, duck pose
- [ ] Red damage tint: `r.setDamageTint(1.0f - (float)p.hp / p.maxHp)` (or scale the model down per hit; pick one)
- [ ] Set `p.height` to the model's real height so the health bar sits right above it

**Behaviour (`Players::update`)**
- [ ] Melee and Ranged walk right from `PLAYER_SPAWN_X` automatically. **Defense never moves**
- [ ] Stop while touching a zombie (`p.engaged`), resume when it dies
- [ ] Attack automatically from the moment they're placed, **even with no enemy in the lane**
- [ ] Ranged: fire projectiles into `w.projectiles` (`fromPlayer = true`, `height = AttackHeight::Mid`). The projectile **animates** and **disappears on hit**. It never harms players
- [ ] **Jump** on W/Up and **duck** on S/Down for every player in a **selected** lane (`w.lanes.selected[p.lane]`). Set `p.pose` (Jumping/Ducking) and `p.y` (jump height). Defense players ignore this
- [ ] All players deal `PLAYER_DAMAGE` (1) per attack
- [ ] Decide what happens at the right edge (stop, or leave the field and get removed)
- [ ] `Players::isProtected`: true while a living Defense player in the same lane is **to the right of** (in front of) this player

**Cards (`PlayerCards.cpp`)**
- [ ] Draw a **mini version of each model** on its card, with the name and cost
- [ ] Locked (`coins < cost`): grayscale (`r.setGrayscale(true)`), **no hover and no click**
- [ ] Unlocked: normal colours, hover effect, clickable
- [ ] Unlock and re-lock happen automatically (already true: `isUnlocked` checks coins every frame)
- [ ] Placement flow (see [§9](#9-interfaces-between-members)). Deduct `cost(type)` from `w.coins` and play `Sfx::Place`

Costs are already set to spec in `Config.h`: Melee 10, Ranged 20 (2×), Defense 30 (3×), start coins 30 (3 melee).

### Member 3: Enemies & combat (`src/enemies/`)

**Models (`Enemies::draw`)**
- [ ] **Easy: upper-ranged** zombie (e.g. one that spits or throws things in an arc from above)
- [ ] **Medium: lower-ranged** zombie (e.g. one that sends something along the ground, or bursting roots/acid from below)
- [ ] **Hard: melee** zombie (big, armoured or fast brute); looks the most dangerous
- [ ] Different **shapes and equipment**, not colour. The design should hint at the attack type and difficulty
- [ ] Walk/run animation, attack animation, red damage tint, `e.height` set correctly
- [ ] Projectile visuals in `Enemies::drawProjectile`

**Behaviour (`Enemies.cpp`, `Combat.cpp`)**
- [ ] Move left at normal speed. **Melee runs** (faster)
- [ ] Attack from the moment they appear, **even with no player in the lane**
- [ ] Upper-ranged attacks are `AttackHeight::High` (the player must **duck**). Lower-ranged attacks are `AttackHeight::Low` (the player must **jump**)
- [ ] Damage: 1 per hit, **melee zombie 2** (`ENEMY_DAMAGE`, `HARD_ENEMY_DAMAGE`)
- [ ] When a player and a zombie touch, **both stop and keep attacking** until one dies
- [ ] Projectile hits: a zombie projectile vs a player whose pose dodges it → no damage. Protected players (`Players::isProtected`) take no damage, and the Defense player in front absorbs the hits instead
- [ ] Player projectiles hit zombies (1 dmg), then `alive = false`
- [ ] Always use `Players::damage` / `Enemies::damage`
- [ ] Zombies stay in their lane. Movement is horizontal only

**Waves (`Waves.cpp`)**: the logic already follows the spec. Tune or polish it:
- [ ] Wave 1: ≥3 easy. Wave 2: ≥5 easy + medium. Wave 3: ≥10 easy + medium + hard. Random lanes
- [ ] A wave ends when its last zombie spawns. The next wave starts only after the field is clear
- [ ] Tune spawn intervals and speeds so the game is beatable but tense

### Member 4: World & cutscene (`src/world/`)

**Environment (`Environment.cpp`)**
- [ ] Themed background: night sky, ruined city or fields, whatever fits the story
- [ ] Lane ground that reads clearly as 4 separate lanes
- [ ] **Home base** at `x ∈ [0, 150]`: detailed (fortified house, bunker, barricaded shop...) and **clearly visible against the environment**, not just a recolour
- [ ] **At least 5 environment props** that fit the theme and sit naturally in the scene: wrecked cars, barricades, streetlights, sandbags, barrels, fences, dead trees, graves...
- [ ] `drawForeground` for anything that should be drawn over characters (fog, rain, flickering light)
- [ ] Animate a little (flickering lamp, moving clouds, fire) using the `time` parameter

**Lanes (`Lanes.cpp`)**: the logic already follows the spec. Polish the look:
- [ ] Hover highlights the **entire lane**
- [ ] Click toggles selected/deselected, multiple lanes at once (done). Make "selected" look clearly different from "hovered"

**Cutscene (`Cutscene.cpp`)**
- [ ] **10–20 s** long (`kDuration`)
- [ ] Shows **at least one player and one zombie, animated and interacting**
- [ ] Establishes **what is being defended and why** (the home base and who is inside)
- [ ] A short **dialogue** between the survivors and the zombie(s), with subtitles via `Text::drawShadowed`. It **must not be text only**
- [ ] Logically leads into the gameplay. The theme, characters and base should match what the player sees next
- [ ] Reuse Member 2's and 3's model drawing functions so the characters look identical (see [§9](#9-interfaces-between-members))
- [ ] Use `setAlpha` and `fullscreen` for fades and camera-like transitions

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
| Member 2 | implements `Players::isProtected(w, p)` and `Players::damage` |
| Member 2 | spawns player projectiles into `w.projectiles` with `fromPlayer = true` |
| Member 3 | `Combat::update` moves **all** projectiles, checks hits and calls `Players::damage` / `Enemies::damage` |
| Member 3 | dodge rule: `High` misses a `Ducking` player, `Low` misses a `Jumping` player, `Mid` always hits |

If you'd rather move player projectiles inside `Players::update`, that's fine. Agree on it so they don't move twice.

### Member 2 ↔ Member 4: placement & lane selection
Current flow (spec section 3.2): **click lane(s) to select them, then click an unlocked card**. One player of that type is placed at the left end of **each selected lane** you can afford.
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

* Start with **30 coins**. Each zombie drops a coin worth **10** (`COIN_DROP_VALUE`), collected when a player touches it.
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
| `enemy_attack` | zombie attacks | **Member 3: add** |
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

**Video (Member 4 edits, 2–5 minutes, with voiceover):**
1. University name and logo
2. Course name and semester (DMET 502 Computer Graphics, Winter 2026)
3. Project name (LAST STAND)
4. Students' names
5. Gameplay: describe the game, its aim and how to play. Show the cutscene, all 3 player and zombie types, jump/duck, unlocking cards, all 3 waves, winning and losing, and all menus including sound settings
6. Thank you

The video may be published online, so make it polished.

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
