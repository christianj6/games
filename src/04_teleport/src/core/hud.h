#pragma once
#include "raylib.h"

class Hud {
public:
  Hud(bool);
  void update(float);
  void draw(Vector3);

private:
  bool debug_mode_enabled;
};
