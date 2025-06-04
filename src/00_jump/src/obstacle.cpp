#include "obstacle.h"
#include "raylib.h"

Obstacle::Obstacle() {
  positionX = 500.0f;
  positionY = 100.0f;
  velocity = 10.0f;
}

void Obstacle::update() {
  // todo
}

void Obstacle::draw() { DrawCircle((int)positionX, (int)positionY, 25, BLUE); }
