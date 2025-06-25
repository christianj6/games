#pragma once
#include "input.h"
#include "raylib.h"
#include <memory>

class IMovemementController {
public:
  virtual ~IMovemementController() = default;
  virtual Vector3 update_movement(float, Vector3, Vector3) = 0;
  virtual Vector2 update_camera() = 0;
  virtual bool get_input_jump() = 0;
};

class PlayerMovementController : public IMovemementController {
public:
  PlayerMovementController(std::unique_ptr<IInputProvider>);
  void set_input_provider(IInputProvider *);
  Vector3 update_movement(float, Vector3, Vector3) override;
  virtual Vector2 update_camera() override;
  virtual bool get_input_jump() override;

private:
  std::unique_ptr<IInputProvider> input_provider;
};

// TODO: enemy movement controller
