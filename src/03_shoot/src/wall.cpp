#include "wall.h"


Wall::Wall(float height, Vector3 position, Color color, float length) : height(height), position(position), color(color), length(length) {}

void Wall::draw() {
  DrawCube(position, 2.0f, height, length, color);
}
