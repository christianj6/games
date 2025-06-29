#pragma once
#include "raylib.h"

class Coin {
  public:
    Coin();
    void update(Vector2 player_position);
    void draw();

  private:
    Vector2 position;
    bool is_collected(Vector2 player_position);
    void reset_position();
};
