#pragma once
#include "raylib.h"
#include "rcamera.h"
#include "world.h"

class Player {
public:
  Player();
  void handle_input();
  void update(float);
  Camera get_camera();
  void set_world(World *w) { world = w; }

private:
  Camera camera;
  World *world = nullptr;
  Vector3 try_move(Vector3 movement) const;
};
