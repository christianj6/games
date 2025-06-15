#include "game.h"
#include "raylib.h"

// TODO: floating ball enemy which hunts the player as in 02_hide game
// TODO: game over if enemy gets too close to the player
// TODO: shooting mechanic which allows the player to shoot the enemy and "win"
// TODO: weapon which allows the player to access the shooting mechanic
// TODO: cmake build system
// TODO: balance, tidy, finish

int main() {
  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI | FLAG_FULLSCREEN_MODE);
  InitWindow(0, 0, "Shoot");
  Game game;

  DisableCursor();
  while (!WindowShouldClose()) {
    game.update();
    game.draw();
  }

  CloseWindow();
  return 0;
}
