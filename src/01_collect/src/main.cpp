#include "coin.h"
#include "enemy.h"
#include "player.h"
#include "raylib.h"

int main() {
  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
  InitWindow(1280, 800, "Coins");

  Player player = Player();
  Enemy enemy = Enemy();
  Coin coin = Coin();
  while (!WindowShouldClose()) // run the loop untill the user presses ESCAPE or
  {
    player.update();
    enemy.update(player.get_position());
    coin.update(player.get_position());

    BeginDrawing();
    ClearBackground(BLACK);
    player.draw();
    enemy.draw();
    coin.draw();
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
