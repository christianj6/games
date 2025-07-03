#include "core/app.h"
#include "raylib.h"

// TODO: improve build instructions for vcpkg
// TODO: polish and finish

// TODO: for next game
/*
 * too dark
 * game feel for movement can be improved
 * health bar would be better
 * better abstractions, more modular and well-architected
 * webassembly build for itchio?
 */

int main() {
  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
  SetTargetFPS(60);
  InitWindow(2460, 1440, "Teleport");
  // disable cursor causes problems with camera in wsl; comment out if needed
  DisableCursor();

  App app{false};
  while (app.run()) {
  }
  CloseWindow();

  return 0;
}
