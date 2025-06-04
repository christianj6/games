#pragma once

class Player {
private:
  float position_x;
  float position_y;
  float velocity;

public:
  Player();
  void update();
  void draw();
};
