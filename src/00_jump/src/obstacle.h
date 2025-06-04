#pragma once

class Obstacle {
public:
  Obstacle();
  void update();
  void draw();

private:
  float positionX;
  float positionY;
  float velocity;
};
