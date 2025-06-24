#include "behaviortree_cpp/basic_types.h"
#include "core/game.h"
#include "fmt/core.h"
#include "raylib.h"
#include <Eigen/Dense>

// TODO: improve build instructions for vcpkg
// TODO: web assembly build for itch.io publish
// SCOPE
/*
 * main idea: same as shoot but more refined
 * game world is conceptually the same as shoot but larger (flat space with
 * pillars) game world is rendered with voxel mesh instancing approach game
 * world has lighting game world has limited visual clutter (eg vines or small
 * plants) player movement is the same as shoot but better abstracted xbox
 * controller input support player can jump player can teleport (simple blink
 * forward mechanic like dishonored) enemies similar to shoot but with better
 * abstractions for managing behavior enemies have real grid-based pathfinding
 * GOAP out of scope: rely on behaviortree_cpp for ai abstractions and just wrap for my needs
 * stop when the main gameplay loop is identical to shoot (hide, teleport, shoot, etc.)
 */

int main() {
  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
  InitWindow(1280, 800, "Hello Raylib");

  while (!WindowShouldClose()) {
    BeginDrawing();

    ClearBackground(BLACK);
    fmt::print("blah");

    // Define a 2x2 matrix of floats
    Eigen::Matrix2f mat;
    mat << 1, 2, 3, 4;

    // Define a 2D vector
    Eigen::Vector2f vec(5, 6);

    // Multiply matrix and vector
    Eigen::Vector2f result = mat * vec;

    DrawText("Hello Raylib", 200, 200, 20, WHITE);

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
