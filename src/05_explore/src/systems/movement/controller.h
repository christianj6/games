#pragma once
#include "input.h"
#include "raylib.h"
#include "systems/ai/behavior.h"
#include <memory>

struct MovementUpdate {
  Vector3 position;
  Vector2 camera;
  bool jump;
  bool blink;
};

class MovementController {
public:
  virtual ~MovementController() = default;
  virtual MovementUpdate tick() = 0;
};

class UserMovementController : public MovementController {
public:
  MovementUpdate tick() override;
  void set_input_provider(std::unique_ptr<InputProvider>);

private:
  std::unique_ptr<InputProvider> input_provider =
      std::make_unique<KeyboardInputProvider>();
  bool using_gamepad_ = false;
};

class AiMovementController : public MovementController {
public:
  MovementUpdate tick() override;

private:
  BehaviorTree behavior_tree;
};
