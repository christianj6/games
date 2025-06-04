#include "enemy.h"
#include "player.h"
#include "raylib.h"

// TODO: add coin that the player can try to pick up

int main() {
  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
  InitWindow(1280, 800, "Coins");

  Player player = Player();
  Enemy enemy = Enemy();
  while (!WindowShouldClose()) // run the loop untill the user presses ESCAPE or
  {
    player.update();
    enemy.update(player.get_position());

    BeginDrawing();
    ClearBackground(BLACK);
    player.draw();
    enemy.draw();
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
