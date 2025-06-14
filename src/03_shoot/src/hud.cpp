#include "hud.h"
#include "raylib.h"

void Hud::draw() {
  int screenWidth = GetScreenWidth();
  int screenHeight = GetScreenHeight();
  DrawCircle(screenWidth / 2, screenHeight / 2, 2, WHITE);
}

void Hud::update(float dt) {
  // TODO
}
