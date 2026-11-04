# LAST STAND: DMET 502 Project 1 (2D zombie lane defense)

The game skeleton is in place and runs end to end: main menu → cutscene → gameplay → pause → win/lose, with sound.
Everything that isn't finished is a **PLACEHOLDER** (plain boxes) in the file of the member who owns it.
Replace your own placeholders. You shouldn't need to touch anyone else's files.

## Build & run

1. Open `Template.sln` in Visual Studio.
2. Set the platform to **x64**. Win32 is not configured.
3. Press F5. The working directory must be the project folder, which is the VS default, so `assets/audio/` is found.

Requirements: OpenGL 3.3 core, GLFW (`lib/glfw3.lib`), GLAD, GLM and miniaudio (`External/include/miniaudio.h`). All of them are already in the repo.

**Adding a new .cpp/.h file:** use *Add → New Item* in VS so it gets registered in `Template.vcxproj`.
Both people who add files will touch that file, so expect small merge conflicts there. Resolve them by keeping both lines.

## Who owns what

| Member | Folder / files | Status |
|---|---|---|
| **1: Core & UI** | `src/main.cpp`, `src/Config.h`, `src/core/*`, `src/game/*` | Done: renderer, text, input, audio, menus, HUD, coins, collectibles, health bars, win/lose |
| **2: Players** | `src/players/Players.*`, `src/players/PlayerCards.*` | Placeholder |
| **3: Enemies & combat** | `src/enemies/Enemies.*`, `Waves.*`, `Combat.*` | Placeholder (basic waves already spawn) |
| **4: World & cutscene** | `src/world/Lanes.*`, `Environment.*`, `Cutscene.*` | Placeholder (lane hover/select already works) |

Each placeholder `.cpp` starts with a **TODO checklist** taken from the spec.

### Rules that keep merges painless
* Only edit files you own. If you need something in someone else's file, ask them.
* In `Players.h` / `Enemies.h`, fields marked **REQUIRED** are read by other code. Don't rename them. Adding fields is fine.
* `Config.h` holds all shared numbers (costs, HP, lane sizes). Tuning values is fine. Tell the team before renaming anything.
* Work on a branch (`git checkout -b players`), then merge or PR into `main`. Pull `main` often.
* **Before pushing, make sure it compiles.** A submission with compile errors gets a zero.

## How the code fits together

```
main.cpp ─ window + loop ─► Game::frame(dt)        (src/game/Game.cpp)
                              │ state machine: MainMenu / SoundSettings / Cutscene / Playing / Paused / Won / Lost
                              ▼ Playing, every frame:
   PlayerCards::update → Lanes::update → Players::update → Waves::update
   → Enemies::update → Combat::update → Collectibles::update → remove dead → check win/lose
                              ▼ draw:
   Environment::drawBackground → Lanes::draw → Environment::drawHomeBase → Collectibles
   → per lane: Players::draw, Enemies::draw → projectiles → Environment::drawForeground
   → health bars (automatic) → HUD (coins, cards, wave)
```

All gameplay state lives in one `World` struct (`src/game/World.h`) that gets passed to every system:
`players`, `enemies`, `projectiles`, `collectibles`, `coins`, `lanes` (Member 4), `cards` (Member 2), `waves` (Member 3).

To remove something, set `alive = false` on it. Core erases dead entries at the end of the frame.

## Coordinates & lanes

* The virtual screen is **1280×720**, with the **origin at the bottom-left and y pointing up**. The window letterboxes automatically.
* There are 4 lanes. Lane 0 is at the top. Use the helpers in `Config.h`: `laneBottomY(l)`, `laneTopY(l)`, `laneCenterY(l)`, `laneGroundY(l)` (where feet stand) and `laneAtY(y)`.
* The home base covers `x ∈ [0, HOME_BASE_RIGHT=150]`. A zombie whose left edge reaches it means **you lose**.
* The HUD covers `y > FIELD_TOP (600)`. Cards sit at `PlayerCards::CARDS_X/Y`.

## Drawing models (everyone)

```cpp
void drawZombie(Renderer2D& r, const Enemy& e) {
    r.push();
    r.translate(e.x, laneGroundY(e.lane));      // local origin = between the feet
    r.setDamageTint(1.0f - (float)e.hp / e.maxHp);  // spec: redder as health drops
    r.rect(-14, 0, 10, 40, Colors::rgb(60, 70, 50));          // legs
    r.roundedRect(-20, 40, 40, 45, 8, Colors::rgb(90, 120, 70)); // torso
    r.push(); r.translate(0, 95); r.rotate(std::sin(e.anim * 3) * 0.1f); // head bob
        r.circle(0, 0, 16, Colors::rgb(140, 170, 110));
    r.pop();
    r.pop();
}
```
Available shapes: `rect, rectCentered, rectGradient, rectHGradient, rectOutline, roundedRect, triangle, quad, circle, ellipse, ring, pie, arc, line, polygon, fullscreen`.
Transforms: `push / pop / translate / rotate / rotateDeg / scale`. Filters (also saved by push/pop): `setDamageTint, setGrayscale, setAlpha`.
Shapes draw in call order, so draw the back parts first. Use `scale(-1, 1)` to mirror a model so it faces left.

**Spec reminder:** no component may be a single primitive. Players, zombies, base and props each need real modelling detail, and the three types must differ in **shape**, not only in colour.

## Other APIs

```cpp
Text::drawShadowed(r, "HELLO", x, y, 20, Colors::White, Align::Center);  // built-in pixel font
UI::button(r, "OK", x, y, w, h)              // returns true when clicked
UI::healthBar(r, cx, y, hp, maxHp)           // already drawn for every player/zombie automatically
Input::keyPressed(GLFW_KEY_W) / keyDown / mousePressed(GLFW_MOUSE_BUTTON_LEFT) / mouse()  // mouse() is in virtual coords
Input::consumeClick()                        // stop later systems reacting to the same click
Audio::play(Sfx::Hit);                       // names in src/core/Audio.h
Collectibles::spawn(w, lane, x, COIN_DROP_VALUE);   // Enemies::damage already does this on death
Players::damage(w, p, n) / Enemies::damage(w, e, n)  // use these, they handle sounds + death
Players::isProtected(w, p)                   // defense rule (Member 2 implements, Combat uses)
```

## Interfaces between members (agree on these early)
* **2 ↔ 3:** Combat (3) calls `Players::damage`, checks `Player::pose` (Jumping dodges `AttackHeight::Low`, Ducking dodges `High`) and `Players::isProtected`. Player projectiles (2) go into `w.projectiles` with `fromPlayer = true`. Combat moves them and applies hits, or Member 2 does that inside `Players::update`. Decide between you.
* **2 ↔ 4:** Placement flow. Right now you select lane(s), then click a card, and one player is placed per selected lane. Jump/duck applies to players in `w.lanes.selected[]`.
* **4 ↔ 2/3:** The cutscene should reuse the same character drawing code so the intro matches gameplay. Expose a `drawModel(r, x, y, anim)` helper from Players/Enemies once your models exist.

## Sound
Files live in `assets/audio/<name>.wav` (`.mp3` and `.flac` also work). The current files are synthesized placeholders made with
`python tools/make_placeholder_sounds.py`. To replace one, drop in a real file with the same name. A missing sound only prints a warning.

## Submission (Member 1)
Zip: `[Team name]-[Member name].zip` containing the source files (.h/.cpp and the project files). Make sure it builds from a clean checkout first.
