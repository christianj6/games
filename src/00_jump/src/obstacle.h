#pragma once
#include <cstddef>

class Obstacle {
public:
  Obstacle(size_t idx = 0);
  void update();
  void draw();
  float get_x() const { return position_x; }
  float get_y() const { return position_y; }

private:
  float position_x;
  float position_y;
  float velocity;
  size_t idx;
};
