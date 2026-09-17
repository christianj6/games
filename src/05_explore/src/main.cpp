// TODO: polish check up
/*
 * generally the blink mechanics need thorough play-testing to identify pain points
 * some blink scenarios can still be a bit tricky so a few more tweaks are welcome
 * pillars should be lit from multiple sides when you turn around to look at them it looks janky
 * maybe increase the landing squash
 * probably need a clamber mechanic ie if you press a button when almost on a ledge you clamber up it
 * then mayber also a dishonoed style blink+clamber is welcome idk
 */

// TODO: friends and home
/*
 * start working with meshes to make the home area look a bit more interesting
 * friend actors with limited animations and behavior trees
 * friends wander around the home chunk, not running into things, and e.g.
 * randomly interact with some objects use mixamo and blender for a simple
 * animation workflow friend ai can be puppeteered by director ai player can
 * talk to friends with a simple text-based system checkpoint: friends roam
 * around the nicer looking home area
 */

// TODO: quests
/*
 * director ai can trigger friends to request items
 * if friend needs something they get an exclamation mark above their head
 * if player interacts with friend while in this state, they will give the
 * player a quest requested item is placed in the world and shown in the hud
 * player can search the open world for the item, collect it, and bring it back
 * checkpoint: friends periodically request items, quests are managed, and
 * player can complete these quests by getting items in the world
 */

// TODO: enemies
/*
 * enemy actors with limited animations and behavior trees
 * enemy behavior tree for stealth gameplay
 * enemies can be made by the director to just stay in a 'guard' state
 * enemy detection works with a timer shown on the enemy like dishonored
 * enemies spawn around quest items to add challenge
 * player can kill enemies by getting close and clicking a kill button
 * enemies can kill player by shooting; game over state
 * feedback: player health bar
 * checkpoint: enemies added to world to make quests more interesting /
 * challenging
 */

// TODO: finish game loop
/*
 * core idea: you go out and get stuff to satisfy the friends,
 * with the goal of ultimately getting all the stuff. when you go out,
 * narrative: you are getting ingredients for a recipe
 * you engage in mini stealth/combat encounters with enemies
 * when you have completed a quest for all friends, you win
 * simple narrative using text communication system
 * playtest and balance, refine
 * checkpoint: core game loop works and is kinda fun
 */

// TODO: polish
/*
 * add sounds
 * camera and movement: better game feel
 * feedback: too dark, movement does not feel nice
 * friends are interesting and fun
 * enemies are challenging and interesting
 * world is interesting to explore
 * game is performant
 * code is clean
 * checkpoint: game is finished
 * ensure ready for kenney game jam
 * stretch: figure out wasm build
 * stretch: expand teleportation mechanics
 */

// TODO: clean up
/*
* change back to main menu starting in app.h
*/

#include "raylib.h"
#include "systems/runtime/app.h"
#if defined(PLATFORM_WEB)
#include <emscripten.h>
#include "systems/movement/input.h"
#endif

// Official raylib web pattern (see examples/core/core_window_web.c):
// keep the desktop-style blocking loop and compile with -sASYNCIFY —
// raylib's WindowShouldClose() yields to the browser each iteration
// (emscripten_sleep(12) in rcore_web.c).

int main() {
  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

  // Initialize with a windowed mode first
  InitWindow(1920, 1080, "Explore");
#ifdef PLATFORM_WEB
  InitWebLookAccumulator();
#else
  // Toggle to fullscreen - more reliable on macOS than FLAG_FULLSCREEN_MODE
  ToggleFullscreen();
  DisableCursor();
#endif

  App app;
#ifdef PLATFORM_WEB
  // Web pointer-lock state machine:
  //  - Lock is requested while gameplay is live, paced because Chrome
  //    rejects re-lock for ~1.25 s after an ESC exit (and without a recent
  //    gesture the request defers to the next click/keypress).
  //  - Lock is released whenever gameplay is not live (menu, pause,
  //    win/lose) so the cursor is visible for menu interaction.
  //  - Chrome swallows the ESC keydown when it exits pointer lock, so an
  //    unexpected lock loss during live gameplay is treated as ESC:
  //    auto-pause. Releases we requested ourselves are flagged to not
  //    trigger it.
  bool pointer_locked = false;  // lock state observed last frame
  bool releasing_lock = false;  // we asked for the release ourselves
  float lock_cooldown = 0.0f;   // paces re-requests after an ESC exit
  while (app.run(false)) {
    const float dt = GetFrameTime();
    bool running = app.is_game_running();
    const bool locked = EM_ASM_INT({
      return document.pointerLockElement === Module.canvas ? 1 : 0;
    });
    if (pointer_locked && !locked && !releasing_lock && running) {
      app.toggle_pause(); // browser exited the lock (ESC / focus loss)
      running = app.is_game_running();
    }
    pointer_locked = locked;
    releasing_lock = false;
    if (running && !locked) {
      if (lock_cooldown <= 0.0f) {
        DisableCursor(); // engages on the next click/keypress (activation)
        lock_cooldown = 1.4f;
      }
    } else if (!running && locked) {
      EnableCursor(); // exits the lock; the cursor becomes visible
      releasing_lock = true;
    }
    if (lock_cooldown > 0.0f) lock_cooldown -= dt;
    ResetWebLookDelta(); // frame's look motion was consumed by app.run()
    WindowShouldClose(); // yields to the browser each iteration via Asyncify
  }
#else
  // Desktop: pure blocking loop (App handles ESC itself).
  while (app.run(false)) {
    WindowShouldClose();
  }
#endif

  CloseWindow();

  return 0;
}
