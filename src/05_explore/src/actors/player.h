#pragma once
#include "actor.h"
#include "raylib.h"
#include "systems/movement/controller.h"
#include <memory>

class Player : public Actor {
public:
  Player();
  MovementUpdate update(float, Blackboard &) override;
  void draw() override;
  Camera3D get_camera() { return camera; }

private:
  Camera3D camera;
  void setup_camera();
};
