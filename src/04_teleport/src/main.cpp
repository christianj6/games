#include "core/app.h"
#include "raylib.h"

// TODO: improve build instructions for vcpkg
// TODO: web assembly build for itch.io publish
// TODO: polish and finish

// SCOPE
/*
 * main idea: same as shoot but more refined
 * xbox controller input support
 * enemies similar to shoot but with better abstractions for managing behavior
 * enemies have real grid-based pathfinding
 * GOAP out of scope: rely on behaviortree_cpp for AI abstractions and just wrap
 * for my needs stop when the main gameplay loop is identical to shoot (hide,
 * teleport, shoot, etc.)
 * game world has limited visual clutter (eg vines or small plants)
 * use mesh instancing systems from before to implement the visual clutter
 */

int main() {
  // TODO: better solution for the wsl windowing issues
  // const int monitor = 0;
  // int width = GetMonitorWidth(monitor);
  // int height = GetMonitorHeight(monitor);
  // int monitorX = GetMonitorPosition(monitor).x;
  // int monitorY = GetMonitorPosition(monitor).y;

  // SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
  InitWindow(2560, 1440, "Teleport");
  // SetWindowPosition(monitorX, monitorY);
  // ToggleBorderlessWindowed();
  // DisableCursor();

  App app{true};
  while (app.run()) {
  }
  CloseWindow();

  return 0;
}
