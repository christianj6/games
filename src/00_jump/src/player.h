#pragma once
#include "obstacle.h"

class Player {
private:
  float position_x;
  float position_y;
  float velocity;

public:
  Player();
  void update();
  void draw();
  bool is_hit(const Obstacle &obstacle) const;
};
