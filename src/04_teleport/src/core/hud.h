// TODO: hud allows for easy toggle of debug information for development
// TODO: rely on fmt for string formatting
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
