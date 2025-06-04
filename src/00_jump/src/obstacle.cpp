#include "obstacle.h"
#include "raylib.h"

Obstacle::Obstacle() {
  position_x = 500.0f;
  position_y = 100.0f;
  velocity = 10.0f;
}

void Obstacle::update() {
  // todo
}

void Obstacle::draw() { DrawCircle((int)position_x, (int)position_y, 25, BLUE); }
