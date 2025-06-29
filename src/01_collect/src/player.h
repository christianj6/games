#pragma once
#include "raylib.h"

class Player {
public:
  Player();
  void update();
  void draw();
  Vector2 get_position();

private:
  Vector2 position;
};

