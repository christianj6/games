#include "world.h"
#include "raylib.h"
#include <memory>

Vector3 get_random_position(float height) {
  return (Vector3){(float)GetRandomValue(-15, 15), height / 2.0f,
                   (float)GetRandomValue(-15, 15)};
}

World::World() : obstacles() {
  const int n = 10;
  for (int i = 0; i < n; i++) {
    float height = GetRandomValue(1, 12);
    Vector3 position = get_random_position(height);
    obstacles.push_back(std::make_unique<Obstacle>(height, position, RED));
  }
}

void World::draw() {
  for (auto &obstacle : obstacles) {
    obstacle->draw();
  }
}
