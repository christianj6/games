#pragma once

class Player {
private:
  float position_x;
  float position_y;
  float velocity;
  /*bool */

public:
  Player();
  void update();
  void draw();
};
