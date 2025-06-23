#include "hud.h"
#include "raylib.h"

void Hud::draw() {
  int screenWidth = GetScreenWidth();
  int screenHeight = GetScreenHeight();
  DrawCircle(screenWidth / 2, screenHeight / 2, 2, WHITE);

  DrawRectangle(10, 10, 250, 133, Fade(SKYBLUE, 0.5f));
  DrawRectangleLines(10, 10, 250, 133, BLUE);

  DrawText("- Move player with WASD", 20, 20, 10, DARKGRAY);
  DrawText("- Move camera with mouse", 20, 40, 10, DARKGRAY);
  DrawText("- Get the blue ball to gain shoot ability", 20, 60, 10, DARKGRAY);
  DrawText("- Shoot with left mouse button", 20, 80, 10, DARKGRAY);
  DrawText("- Shoot all enemies to win", 20, 100, 10, DARKGRAY);
  DrawText("- Game over if enemy touches you", 20, 120, 10, DARKGRAY);
}

void Hud::update(float dt) {
  // TODO
}
