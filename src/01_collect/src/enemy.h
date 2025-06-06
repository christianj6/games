#pragma once
#include "raylib.h"


class Enemy {
public:
  Enemy();
  void update(Vector2 player_position);
  void draw();

private:
  Vector2 position;
};

