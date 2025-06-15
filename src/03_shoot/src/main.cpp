#include "game.h"
#include "raylib.h"

// TODO: floating ball enemy which hunts the player as in 02_hide game
// TODO: game over if enemy gets too close to the player
// TODO: shooting mechanic which allows the player to shoot the enemy and "win"
// TODO: weapon which allows the player to access the shooting mechanic
// TODO: cmake build system
// TODO: balance, tidy, finish

int main() {
  const int monitor = 0;
  int width = GetMonitorWidth(monitor);
  int height = GetMonitorHeight(monitor);

  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
  InitWindow(width, height, "Shoot");

  // Center window on the current monitor
  int monitorX = GetMonitorPosition(monitor).x;
  int monitorY = GetMonitorPosition(monitor).y;
  SetWindowPosition(monitorX, monitorY);
  ToggleBorderlessWindowed();

  Game game;
  DisableCursor();

  while (!WindowShouldClose()) {
    game.update();
    game.draw();
  }

  CloseWindow();
  return 0;
}
