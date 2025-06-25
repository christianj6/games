#pragma once
#include "raylib.h"

class IInputProvider {
public:
  virtual ~IInputProvider() = default;
  virtual Vector3 get_input_movement_vector() = 0;
  // TODO: functions for other types of input
};

class KeyboardInputProvider : public IInputProvider {
public:
  KeyboardInputProvider() {};
  Vector3 get_input_movement_vector() override;
};

class ControllerInputProvider : public IInputProvider {
public:
  Vector3 get_input_movement_vector() override;
};
