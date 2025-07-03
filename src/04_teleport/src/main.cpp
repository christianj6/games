#include "core/app.h"
#include "raylib.h"

// TODO: polish and finish

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
