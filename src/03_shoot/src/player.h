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
  // getter returns reference to the container so there are no copies
  std::vector<Projectile> &get_projectiles() { return projectiles; }
  void enable_weapon() { has_weapon = true; }

private:
  Camera camera;
  World *world = nullptr;
  Vector3 try_move(Vector3 movement) const;
  // just store the projectiles on the character (shooting is tied to camera)
  std::vector<Projectile> projectiles;
  bool has_weapon = false;
};
