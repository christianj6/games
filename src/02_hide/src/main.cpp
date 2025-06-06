#include "player.h"
#include "raylib.h"

int main() {
  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
  InitWindow(1280, 800, "Hide");

  // TODO: procedurally generated game world
  auto player = Player();

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);
    player.draw();
    EndDrawing();
  }
  CloseWindow();
  return 0;
}
