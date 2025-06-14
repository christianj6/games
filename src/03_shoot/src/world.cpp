#include "world.h"
#include "raylib.h"
#include <memory>
#include <random>

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
  // TODO: improve obstacle initialization
  const int n = 10;
  for (int i = 0; i < n; i++) {
    float height = GetRandomValue(1, 12);
    Vector3 position = get_random_position(height);
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
