#include "hud.h"
#include "raylib.h"


void Hud::draw(Blackboard blackboard) {
  DrawFPS(20, 40);
  DrawCircle(GetScreenWidth() / 2, GetScreenHeight() / 2, 2, BLACK);
}
