#pragma once
#include "raylib.h"


class Enemy {
public:
  Enemy();
  bool update(Vector2 player_position);
  void draw();

private:
  Vector2 position;
  bool hit_player = false;
};

