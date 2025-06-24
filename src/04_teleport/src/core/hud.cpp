#include "hud.h"
#include "raylib.h"

void Hud::draw() {
  int screenWidth = GetScreenWidth();
  int screenHeight = GetScreenHeight();

  // reticle
  DrawCircle(screenWidth / 2, screenHeight / 2, 2, WHITE);

  // instruction card
  DrawRectangle(10, 10, 250, 133, Fade(SKYBLUE, 0.5f));
  DrawRectangleLines(10, 10, 250, 133, BLUE);

  // debug info
  DrawFPS(15, 15);
}

void Hud::update(float dt) {
  // TODO: toggleable debug mode
}
