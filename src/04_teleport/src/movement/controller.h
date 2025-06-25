#pragma once
#include "input.h"
#include "raylib.h"
#include <memory>

class IMovemementController {
public:
  virtual ~IMovemementController() = default;
  virtual Vector3 update_movement(float, Vector3, Vector3) = 0;
};

class PlayerMovementController : public IMovemementController {
public:
  PlayerMovementController(std::unique_ptr<IInputProvider>);
  void set_input_provider(IInputProvider *);
  Vector3 update_movement(float, Vector3, Vector3) override;

private:
  std::unique_ptr<IInputProvider> input_provider;
};

// TODO: enemy movement controller
