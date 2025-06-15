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

private:
  std::vector<std::unique_ptr<Obstacle>> obstacles;
  std::vector<std::unique_ptr<Wall>> walls;
};
