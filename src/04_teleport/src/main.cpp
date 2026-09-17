#include "core/app.h"
#include "raylib.h"
#if defined(PLATFORM_WEB)
#include "movement/input.h"
#include <emscripten.h>
#endif

// Official raylib web pattern (see examples/core/core_window_web.c):
// keep the desktop-style blocking loop and compile with -sASYNCIFY —
// raylib's WindowShouldClose() yields to the browser each iteration
// (emscripten_sleep(12) in rcore_web.c).

int main() {
  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
  SetTargetFPS(60);
  InitWindow(2460, 1440, "Teleport");
#ifndef PLATFORM_WEB
  // disable cursor causes problems with camera in wsl; comment out if needed
  DisableCursor();
#endif

  App app{false};
#ifdef PLATFORM_WEB
  InitWebLookAccumulator();
  // Web pointer-lock state machine (same approach as 05_explore):
  //  - Lock is requested while gameplay is live, paced because Chrome
  //    rejects re-lock for ~1.25 s after an ESC exit (and without a recent
  //    gesture the request defers to the next click/keypress).
  //  - Lock is released whenever gameplay is not live (menu, pause,
  //    win/lose) so the cursor is visible for menu interaction.
  //  - Chrome swallows the ESC keydown when it exits pointer lock, so an
  //    unexpected lock loss during live gameplay is treated as ESC: pause.
  //    Releases we requested ourselves are flagged to not trigger it.
  bool pointer_locked = false; // lock state observed last frame
  bool releasing_lock = false; // we asked for the release ourselves
  float lock_cooldown = 0.0f;  // paces re-requests after an ESC exit
  while (app.run()) {
    const float dt = GetFrameTime();
    bool playing = app.state() == AppState::GAME_PLAY;
    const bool locked = EM_ASM_INT(
        { return document.pointerLockElement === Module.canvas ? 1 : 0; });
    if (pointer_locked && !locked && !releasing_lock && playing) {
      app.pause(); // browser exited the lock (ESC / focus loss)
      playing = false;
    }
    pointer_locked = locked;
    releasing_lock = false;
    if (playing && !locked) {
      if (lock_cooldown <= 0.0f) {
        DisableCursor(); // engages on the next click/keypress (activation)
        lock_cooldown = 1.4f;
      }
    } else if (!playing && locked) {
      EnableCursor(); // exits the lock; the cursor becomes visible
      releasing_lock = true;
    }
    if (lock_cooldown > 0.0f)
      lock_cooldown -= dt;
    ResetWebLookDelta(); // frame's look motion was consumed by app.run()
    WindowShouldClose(); // yields to the browser each iteration via Asyncify
  }
#else
  while (app.run()) {
  }
#endif
  CloseWindow();

  return 0;
}
