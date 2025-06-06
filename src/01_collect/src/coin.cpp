#include "coin.h"
#include "raylib.h"

Coin::Coin() { reset_position(); }

void Coin::update(Vector2 player_position) {
  if (is_collected(player_position)) {
    reset_position();
  }
}

void Coin::reset_position() {
  int width = GetScreenWidth();
  int height = GetScreenHeight();
  float x = GetRandomValue(0, width);
  float y = GetRandomValue(0, height);
  position = {x, y};
}

void Coin::draw() { DrawCircle(position.x, position.y, 30, YELLOW); }

bool Coin::is_collected(Vector2 player_position) {
  return CheckCollisionCircles(position, 30, player_position, 50);
}
