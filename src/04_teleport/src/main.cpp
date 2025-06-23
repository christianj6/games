#include "raylib.h"

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
