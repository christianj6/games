#pragma once
#include "projectile.h"
#include "raylib.h"
#include "rcamera.h"
#include "world.h"
#include <vector>

class Player {
public:
  Player();
  void handle_input();
  Vector3 update(float);
  void draw();
  Camera get_camera();
  void set_world(World *w) { world = w; }

private:
  Camera camera;
  World *world = nullptr;
  Vector3 try_move(Vector3 movement) const;
  std::vector<Projectile> projectiles;
};
