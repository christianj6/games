#pragma once
#include "raylib.h"
#include "rcamera.h"

class Player {
public:
  Player();
  void handle_input();
  void update(float);
  Camera get_camera();

private:
  Camera camera;
};
