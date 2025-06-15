#include "world.h"
#include "raylib.h"
#include "raymath.h"
#include <memory>
#include <random>

const float playing_field_size = 100.f;

bool check_obstacle_collision(const Vector3 &pos1, float height1,
                              const Vector3 &pos2, float height2) {
  // Define collision box dimensions
  const float width = 2.0f; // Standard width for obstacles
  float radius1 = width / 2.0f;
  float radius2 = width / 2.0f;

  // Check for cylinder-cylinder intersection
  float dx = pos1.x - pos2.x;
  float dz = pos1.z - pos2.z;
  float distance = sqrt(dx * dx + dz * dz);

  return distance < (radius1 + radius2);
}

Vector3 get_random_position(float height) {
  const float margin = 5.0f;
  const float min_pos = (-playing_field_size / 2.0f) + margin;
  const float max_pos = (playing_field_size / 2.0f) - margin;

  return (Vector3){(float)GetRandomValue((int)min_pos, (int)max_pos),
                   height / 2.0f,
                   (float)GetRandomValue((int)min_pos, (int)max_pos)};
}

Color get_random_color() {
  std::vector<Color> colors = {GRAY, RAYWHITE, DARKPURPLE, BLACK};

  static std::mt19937 gen(std::random_device{}());
  std::uniform_int_distribution<> dist(0, colors.size() - 1);
  return colors[dist(gen)];
}

World::World() : obstacles(), walls(), enemies() {
  // obstacles
  const int n = 100;
  const int max_attempts = 100; // Maximum attempts to place each obstacle
  for (int i = 0; i < n; i++) {
    float height = GetRandomValue(1, 12);
    Vector3 position;
    bool valid_position = false;

    // Try to find a non-colliding position
    for (int attempt = 0; attempt < max_attempts; attempt++) {
      position = get_random_position(height);
      valid_position = true;

      // Check against all existing obstacles
      for (const auto &obstacle : obstacles) {
        if (check_obstacle_collision(position, height, obstacle->get_position(),
                                     obstacle->get_height())) {
          valid_position = false;
          break;
        }
      }

      if (valid_position)
        break;
    }

    Color color = get_random_color();
    obstacles.push_back(std::make_unique<Obstacle>(height, position, color));
  }
  // Add some enemies
  const int num_enemies = 3;
  for (int i = 0; i < num_enemies; i++) {
    enemies.push_back(std::make_unique<Enemy>());
  }

  // walls
  const float wall_height = 50.0f;
  const Color wall_color = DARKGRAY;
  const float half_size = playing_field_size / 2.0f;

  // North wall (along Z axis)
  walls.push_back(std::make_unique<Wall>(
      wall_height, Vector3{0.0f, wall_height / 2, -half_size}, wall_color,
      playing_field_size,
      true // rotated 90 degrees
      ));

  // South wall (along Z axis)
  walls.push_back(std::make_unique<Wall>(
      wall_height, Vector3{0.0f, wall_height / 2, half_size}, wall_color,
      playing_field_size,
      true // rotated 90 degrees
      ));

  // East wall (along X axis)
  walls.push_back(std::make_unique<Wall>(
      wall_height, Vector3{half_size, wall_height / 2, 0.0f}, wall_color,
      playing_field_size));

  // West wall (along X axis)
  walls.push_back(std::make_unique<Wall>(
      wall_height, Vector3{-half_size, wall_height / 2, 0.0f}, wall_color,
      playing_field_size));
}

void World::draw() {
  DrawPlane((Vector3){0.0f, 0.0f, 0.0f},
            (Vector2){playing_field_size, playing_field_size}, LIGHTGRAY);
  for (auto &obstacle : obstacles) {
    obstacle->draw();
  }
  for (auto &wall : walls) {
    wall->draw();
  }
  for (auto &enemy : enemies) {
    enemy->draw();
  }
}

CollisionInfo World::check_collision(const Vector3 &position) const {
  const float PLAYER_RADIUS = 0.5f;
  CollisionInfo result = {false, {0, 0, 0}};

  // Check wall collisions
  for (const auto &wall : walls) {
    Vector3 wall_pos = wall->position;
    if (wall->rotate90) {
      // Wall along X axis
      float dist_z = position.z - wall_pos.z;
      float dist_x = fabs(position.x - wall_pos.x);
      if (fabs(dist_z) < (1.0f + PLAYER_RADIUS) &&
          dist_x < (wall->length / 2 + PLAYER_RADIUS)) {
        result.collision = true;
        result.normal = {0, 0, (dist_z > 0) ? 1.0f : -1.0f};
        return result;
      }
    } else {
      // Wall along Z axis
      float dist_x = position.x - wall_pos.x;
      float dist_z = fabs(position.z - wall_pos.z);
      if (fabs(dist_x) < (1.0f + PLAYER_RADIUS) &&
          dist_z < (wall->length / 2 + PLAYER_RADIUS)) {
        result.collision = true;
        result.normal = {(dist_x > 0) ? 1.0f : -1.0f, 0, 0};
        return result;
      }
    }
  }

  // Check obstacle collisions
  for (const auto &obstacle : obstacles) {
    Vector3 obs_pos = obstacle->get_position();
    float dx = position.x - obs_pos.x;
    float dz = position.z - obs_pos.z;
    float distance = sqrt(dx * dx + dz * dz);
    if (distance < (1.0f + PLAYER_RADIUS)) {
      result.collision = true;
      // Calculate normal from obstacle center to player
      result.normal = Vector3Normalize({dx, 0, dz});
      return result;
    }
  }

  return result;
}

void World::update(float dt) {
  for (auto &enemy : enemies) {
    enemy->update(dt);
  }
}
