#pragma once
#include "enemy.h"
#include "obstacle.h"
#include "raylib.h"
#include "wall.h"
#include <memory>
#include <vector>

struct CollisionInfo {
  bool collision;
  Vector3 normal;
};

class World {
public:
  World();
  void update(float, const Vector3 &);
  void draw();
  CollisionInfo check_collision(const Vector3 &position) const;

private:
  std::vector<std::unique_ptr<Obstacle>> obstacles;
  std::vector<std::unique_ptr<Wall>> walls;
  std::vector<std::unique_ptr<Enemy>> enemies;
};
