#include "obstacle.h"
#include "raylib.h"
#include <cstddef>
#include <cstdio>

Obstacle::Obstacle(size_t idx) {
  float random_offset = GetRandomValue(-100, 100);
  position_x = 300.0f * (1 + idx) + 1000 + random_offset;
  position_y = 700.0f;
  velocity = 5.0f;
}

void Obstacle::update() {
  position_x -= velocity; // Move left

  // Reset position when off screen
  if (position_x < -50.0f) { // -50 to account for obstacle size
    position_x =
        GetScreenWidth() + 50.0f; // Reset to just off right side of screen
  }
}

void Obstacle::draw() {
  DrawCircle((int)position_x, (int)position_y, 25, BLUE);
}
