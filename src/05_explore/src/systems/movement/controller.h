#pragma once
#include "input.h"
#include "raylib.h"
#include <memory>

struct MovementUpdate {
  Vector3 position;
  Vector2 camera;
  bool jump;
  bool blink;
  bool jump_held;
  bool sprint;
  bool blink_held; // true every frame the blink button is held down
  bool recall;         // LB pressed (edge) — kept for compatibility
  bool recall_held;    // LB held down (level)
  bool place_anchor;   // LB+RB chord
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

