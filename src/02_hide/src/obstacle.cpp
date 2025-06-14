#include "obstacle.h"

Obstacle::Obstacle(Vector2 starting_position) : GameObject(starting_position) {}

void Obstacle::draw() { DrawCircle(position.x, position.y, 120, GRAY); }

void Obstacle::update(float dt, bool allow_movement) {}

std::vector<Obstacle> get_world_obstacles(int n) {
  std::vector<Obstacle> obstacles;

  const float MIN_RADIUS = 600.0f;        // Minimum distance from spawn point
  const float MAX_RADIUS = 6000.0f;       // Maximum distance from spawn point
  const float TWO_PI = 6.28318f;          // 2 * PI
  const float SAFE_SPAWN_RADIUS = 500.0f; // Safe zone around spawn point (0,0)

  // Create n obstacles in a more organic pattern around spawn point at (0,0)
  while (obstacles.size() < n) {
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

    // Calculate final position
    Vector2 pos = {x + noise_x, y + noise_y};

    // Check if position is far enough from center after noise
    float dist_from_center = sqrtf(pos.x * pos.x + pos.y * pos.y);
    if (dist_from_center >= SAFE_SPAWN_RADIUS) {
      obstacles.push_back(Obstacle(pos));
    }
  }

  return obstacles;
}
