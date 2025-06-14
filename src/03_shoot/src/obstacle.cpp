#include "obstacle.h"

Obstacle::Obstacle(float height, Vector3 position, Color color)
    : height(height), position(position), color(color) {}

void Obstacle::draw() { DrawCube(position, 2.0f, height, 2.0f, color); }
