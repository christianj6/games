#include "fmt/core.h"
#include "raylib.h"
#include <Eigen/Dense>
#include "core/game.h"

// TODO: improve build instructions for vcpkg
// TODO: web assembly build for itch.io publish

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
