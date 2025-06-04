#pragma once

class Obstacle {
public:
  Obstacle();
  void update();
  void draw();

private:
  float position_x;
  float position_y;
  float velocity;
};
