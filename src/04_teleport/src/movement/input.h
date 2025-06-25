#pragma once
#include "raylib.h"

class IInputProvider {
public:
  virtual ~IInputProvider() = default;
  virtual Vector3 get_input_movement_vector() = 0;
  virtual Vector2 get_input_look_vector() = 0;
  virtual bool get_input_jump() = 0;
  virtual bool get_input_action() = 0;
};

class KeyboardInputProvider : public IInputProvider {
public:
  KeyboardInputProvider() {};
  Vector3 get_input_movement_vector() override;
  Vector2 get_input_look_vector() override;
  bool get_input_jump() override;
  bool get_input_action() override;
};

class ControllerInputProvider : public IInputProvider {
public:
  Vector3 get_input_movement_vector() override;
  Vector2 get_input_look_vector() override;
  bool get_input_jump() override;
  bool get_input_action() override;
};
