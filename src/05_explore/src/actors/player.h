#pragma once
#include "actor.h"
#include "raylib.h"
#include "systems/movement/controller.h"

class Player : public Actor {
public:
  Player();
  MovementUpdate update(float, Blackboard &) override;
  void draw() override;
  Camera3D get_camera() { return camera; }

private:
  Camera3D camera;
  void setup_camera();
  Vector3 adjust_movement_relative_to_camera(float, Vector3);
};
