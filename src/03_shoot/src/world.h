#pragma once
#include "enemy.h"
#include "obstacle.h"
#include "raylib.h"
#include "wall.h"
#include <memory>
#include <vector>

#include "collision_checker.h"

class World : public CollisionChecker {
public:
  World();
  void update(float, const Vector3 &);
  void draw();
  CollisionInfo check_collision(const Vector3 &position) const;
  CollisionInfo check_collision_ray(Ray ray, float max_distance) const;
  const std::vector<std::unique_ptr<Enemy>> &get_enemies() const {
    return enemies;
  }

private:
  std::vector<std::unique_ptr<Obstacle>> obstacles;
  std::vector<std::unique_ptr<Wall>> walls;
  std::vector<std::unique_ptr<Enemy>> enemies;
};
