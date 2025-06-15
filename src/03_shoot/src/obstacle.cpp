#include "obstacle.h"
#include "raylib.h"

Obstacle::Obstacle(float height, Vector3 position, Color color)
    : height(height), position(position), color(color) {}

void Obstacle::draw() {
  // Draw the main cube
  DrawCube(position, 2.0f, height, 2.0f, color);

  // Draw the edges in a darker color
  Color lineColor = BLACK;
  lineColor.a = 255; // Ensure lines are fully opaque

  // Draw wireframe cube slightly larger to prevent z-fighting
  DrawCubeWires(position, 2.0f, height, 2.0f, lineColor);
}

Vector3 Obstacle::get_position() { return position; }
float Obstacle::get_height() { return height; }
