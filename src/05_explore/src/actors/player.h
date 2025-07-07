#pragma once
#include "actor.h"
#include "raylib.h"

class Player : public Actor {
public:
  Player();
  Vector3 update(float, Blackboard&) override;
  void draw() override;
  Camera3D get_camera() {return camera;}
private:
  Camera3D camera;
};
