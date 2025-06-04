#include "player.h"
#include "raylib.h"
#include "resource_dir.h" // utility header for SearchAndSetResourceDir

int main() {
  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
  InitWindow(1280, 800, "Coins");
  SearchAndSetResourceDir("resources");
  Texture wabbit = LoadTexture("wabbit_alpha.png");

  Player player = Player();
  while (!WindowShouldClose()) // run the loop untill the user presses ESCAPE or
  {
    player.update();

    BeginDrawing();
    ClearBackground(BLACK);
    player.draw();
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
