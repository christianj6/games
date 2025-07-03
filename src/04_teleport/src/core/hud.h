#pragma once
#include "raylib.h"

class Hud {
public:
  Hud(bool);
  void update(float);
  void draw(Vector3, int);

private:
  bool debug_mode_enabled;
};
