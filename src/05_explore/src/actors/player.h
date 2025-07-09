#pragma once
#include "actor.h"
#include "raylib.h"
#include "systems/movement/controller.h"
#include <memory>

class Player : public Actor {
public:
  Player();
  Vector3 update(float, Blackboard &) override;
  void draw() override;
  Camera3D get_camera() { return camera; }

private:
  Camera3D camera;
  void setup_camera();
  // std::unique_ptr<MovementController> movement_controller =
  //     std::make_unique<UserMovementController>();
};
