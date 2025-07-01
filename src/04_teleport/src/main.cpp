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
  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
  SetTargetFPS(60);
  InitWindow(2460, 1440, "Teleport");
  DisableCursor();

  App app{true};
  while (app.run()) {
  }
  CloseWindow();

  return 0;
}
