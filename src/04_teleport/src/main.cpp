#include "core/app.h"

// TODO: improve build instructions for vcpkg
// TODO: web assembly build for itch.io publish
// TODO: polish and finish

// SCOPE
/*
 * main idea: same as shoot but more refined
 * xbox controller input support
 * player can jump
 * player can teleport (simple blink forward mechanic like Dishonored)
 * enemies similar to shoot but with better abstractions for managing behavior
 * enemies have real grid-based pathfinding
 * GOAP out of scope: rely on behaviortree_cpp for AI abstractions and just wrap
 * for my needs stop when the main gameplay loop is identical to shoot (hide,
 * teleport, shoot, etc.)
 * game world has limited visual clutter (eg vines or small plants)
 * use mesh instancing systems from before to implement the visual clutter
 */

int main() {
  const int monitor = 0;
  int width = GetMonitorWidth(monitor);
  int height = GetMonitorHeight(monitor);
  int monitorX = GetMonitorPosition(monitor).x;
  int monitorY = GetMonitorPosition(monitor).y;

  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
  InitWindow(width, height, "Teleport");
  SetWindowPosition(monitorX, monitorY);
  ToggleBorderlessWindowed();
  DisableCursor();

  App app{true};
  while (app.run()) {
  }
  CloseWindow();

  return 0;
}
