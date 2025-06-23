#include "wall.h"

Wall::Wall(float height, Vector3 position, Color color, float length,
           bool rotate90)
    : height(height), position(position), color(color), length(length),
      rotate90(rotate90) {}

void Wall::draw() {
  if (rotate90) {
    DrawCube(position, length, height, 2.0f, color);
  } else {
    DrawCube(position, 2.0f, height, length, color);
  }
}
