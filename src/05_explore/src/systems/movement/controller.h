#pragma once
#include "raylib.h"
#include "input.h"
#include "systems/ai/behavior.h"

struct MovementUpdate {
  Vector3 position;
  Vector2 camera;
  bool jump;
  bool blink;
};

class MovementController {
  public:
    virtual MovementUpdate tick() = 0;
};

class UserMovementController : public MovementController {
  public:
    MovementUpdate tick();
  private:
    InputProvider input_provider;
};

class AiMovementController : public MovementController {
  public:
    MovementUpdate tick();
  private:
    BehaviorTree behavior_tree;
};
