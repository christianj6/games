#include "game.h"
#include "raylib.h"
#if defined(PLATFORM_WEB)
#include <emscripten.h>
#endif

int main() {
  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
#ifndef PLATFORM_WEB
  SetTargetFPS(60);
#endif
  InitWindow(2460, 1440, "Shoot");

  Game game;
#ifndef PLATFORM_WEB
  // disable cursor causes problems with camera in wsl; comment out if needed
  // DisableCursor();
  while (!WindowShouldClose()) {
    game.update();
    game.draw();
  }
#else
  // Web pointer-lock state machine (see 05_explore): mouse-look needs pointer
  // lock, so request it while gameplay is live and release on win/lose so the
  // cursor is visible again. The game has no pause concept, so an unexpected
  // lock loss (ESC / focus loss) simply drops mouse-look until the next click
  // re-engages the lock. Chrome rejects re-lock for ~1.25 s after an ESC exit,
  // hence the paced re-request.
  float lock_cooldown = 0.0f;
  while (!WindowShouldClose()) {
    const float dt = GetFrameTime();
    const bool running = game.is_running();
    const bool locked = EM_ASM_INT(
        { return document.pointerLockElement === Module.canvas ? 1 : 0; });
    if (running && !locked) {
      if (lock_cooldown <= 0.0f) {
        DisableCursor(); // engages on the next click/keypress (activation)
        lock_cooldown = 1.4f;
      }
    } else if (!running && locked) {
      EnableCursor(); // exits the lock; the cursor becomes visible
    }
    if (lock_cooldown > 0.0f)
      lock_cooldown -= dt;
    game.update();
    game.draw();
    WindowShouldClose(); // yields to the browser each iteration via Asyncify
  }
#endif

  CloseWindow();
  return 0;
}
