#pragma once
#include "obstacle.h"
#include "wall.h"
#include <memory>
#include <vector>

class World {
public:
  World();
  void update(float);
  void draw();
  bool check_collision(const Vector3 &position) const;

private:
  std::vector<std::unique_ptr<Obstacle>> obstacles;
  std::vector<std::unique_ptr<Wall>> walls;
};
