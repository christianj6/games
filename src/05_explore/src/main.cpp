#include "raylib.h"
#include "systems/runtime/app.h"
#if defined(PLATFORM_WEB)
#include "systems/movement/input.h"
#include <emscripten.h>
#endif

// Official raylib web pattern (see examples/core/core_window_web.c):
// keep the desktop-style blocking loop and compile with -sASYNCIFY —
// raylib's WindowShouldClose() yields to the browser each iteration
// (emscripten_sleep(12) in rcore_web.c).
// On desktop these are immediately superseded by ToggleFullscreen(); they
// mainly set the web canvas aspect.
namespace {
constexpr int kWindowWidth = 1920;
constexpr int kWindowHeight = 1080;
// Chrome rejects re-lock for ~1.25 s after an ESC exit; margin on top.
constexpr float kLockCooldown = 1.4f;
} // namespace

int main() {
  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

  InitWindow(kWindowWidth, kWindowHeight, "Explore");
#ifdef PLATFORM_WEB
  init_web_look_accumulator();
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
  bool pointer_locked = false; // lock state observed last frame
  bool releasing_lock = false; // we asked for the release ourselves
  float lock_cooldown = 0.0f;  // paces re-requests after an ESC exit
  while (app.run()) {
    const float dt = GetFrameTime();
    bool running = app.is_game_running();
    // The EM_ASM body is JavaScript: clang-format would rewrite `===` as `==
    // =`.
    // clang-format off
    const bool locked = EM_ASM_INT(
        { return document.pointerLockElement === Module.canvas ? 1 : 0; });
    // clang-format on
    if (pointer_locked && !locked && !releasing_lock && running) {
      app.toggle_pause(); // browser exited the lock (ESC / focus loss)
      running = app.is_game_running();
    }
    pointer_locked = locked;
    releasing_lock = false;
    if (running && !locked) {
      if (lock_cooldown <= 0.0f) {
        DisableCursor(); // engages on the next click/keypress (activation)
        lock_cooldown = kLockCooldown;
      }
    } else if (!running && locked) {
      EnableCursor(); // exits the lock; the cursor becomes visible
      releasing_lock = true;
    }
    if (lock_cooldown > 0.0f)
      lock_cooldown -= dt;
    reset_web_look_delta(); // frame's look motion was consumed by app.run()
    WindowShouldClose();    // yields to the browser each iteration via Asyncify
  }
#else
  // Desktop: pure blocking loop (App handles ESC itself).
  while (app.run()) {
    WindowShouldClose();
  }
#endif

  CloseWindow();

  return 0;
}
