#include "raylib.h"

// TODO: more modular abstractions for colliders, controllers, etc.
// TODO: abstraction layer for collision detection and vector operations
// TODO: files are organized into directories for different layers / systems
// TODO: slightly larger game world, made possible through voxel-based rendering
// TODO: pseudo voxel world: implement a world state with Matrix, abstractions update this
// TODO: voxels are either visible or not; simple "exposure" detection based on surrounding blocks and open space
// TODO: rely on raylib's mesh instancing to draw them; a reasonable start
// TODO: cache visibility to avoid repeated calculations
// TODO: hud allows for easy toggle of debug information for development
// TODO: xbox controller support
// TODO: teleportation mechanic allows player to blink forwards and on top of obstacles
// TODO: jump mechanic which pairs with teleportation
// TODO: world layout regularly changes like a shifting labyrinth
// TODO: enemies have real grid-based pathfinding for more intense hunting of the player
// TODO: enemies are more interesting or there exist additional "creates" with cooler visuals
// TODO: consider throwing in some visual clutter and additional lighting effects to make the game world pop
// TODO: web assembly build?


int main() {
  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
  InitWindow(1280, 800, "Hello Raylib");

  while (!WindowShouldClose()) {
    BeginDrawing();

    ClearBackground(BLACK);

    DrawText("Hello Raylib", 200, 200, 20, WHITE);

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
