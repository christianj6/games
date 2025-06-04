#pragma once

class Player {
private:
  float positionX;
  float positionY;
  float velocity;

public:
  Player();
  void update();
  void draw();
};
