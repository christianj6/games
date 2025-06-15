#include "world.h"
#include "raylib.h"
#include <memory>
#include <random>


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
  return (Vector3){(float)GetRandomValue(-15, 15), height / 2.0f,
                   (float)GetRandomValue(-15, 15)};
}

Color get_random_color() {
  std::vector<Color> colors = {BLUE, RED, YELLOW};

  static std::mt19937 gen(std::random_device{}());
  std::uniform_int_distribution<> dist(0, colors.size() - 1);
  return colors[dist(gen)];
}

World::World() : obstacles() {
  const int n = 10;
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
}

void World::draw() {
  DrawPlane((Vector3){0.0f, 0.0f, 0.0f}, (Vector2){32.0f, 32.0f}, LIGHTGRAY);
  for (auto &obstacle : obstacles) {
    obstacle->draw();
  }
}

void World::update(float dt) {
  // TODO
}
