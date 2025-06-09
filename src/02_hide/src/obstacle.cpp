#include "obstacle.h"

Obstacle::Obstacle(Vector2 starting_position) : GameObject(starting_position) {}

void Obstacle::draw() { DrawCircle(position.x, position.y, 120, GRAY); }

void Obstacle::update(float dt) {}

std::vector<Obstacle> get_world_obstacles(int n) {
  std::vector<Obstacle> obstacles;

  const float MIN_RADIUS = 300.0f;  // Minimum distance from center
  const float MAX_RADIUS = 2000.0f; // Maximum distance from center
  const float TWO_PI = 6.28318f;    // 2 * PI

  // Create n obstacles in a more organic pattern
  for (int i = 0; i < n; i++) {
    // Use polar coordinates for more natural distribution
    float angle = GetRandomValue(0, 1000) * (TWO_PI / 1000.0f); // Random angle
    float radius = MIN_RADIUS + GetRandomValue(0, 1000) *
                                    ((MAX_RADIUS - MIN_RADIUS) / 1000.0f);

    // Convert polar to cartesian coordinates
    float x = radius * cosf(angle);
    float y = radius * sinf(angle);

    // Add some noise to break up perfect circles
    float noise_x = GetRandomValue(-200, 200);
    float noise_y = GetRandomValue(-200, 200);

    obstacles.push_back(Obstacle(Vector2{x + noise_x, y + noise_y}));
  }

  return obstacles;
}
