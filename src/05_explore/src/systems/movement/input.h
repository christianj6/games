#pragma once
#include "raylib.h"

class InputProvider {
public:
  InputProvider() = default;
  ~InputProvider() = default;
  virtual Vector3 get_input_movement_vector() = 0;
  virtual Vector2 get_input_look_vector() = 0;
  virtual bool get_input_jump() = 0;
  virtual bool get_input_jump_held() = 0;
  virtual bool get_input_sprint() = 0;
  virtual bool get_input_blink() = 0;
  virtual bool get_input_blink_held() = 0;
  virtual bool get_input_recall() = 0;
  // virtual bool get_input_attack() = 0;
  // virtual bool get_input_pause() = 0;
  // virtual bool get_input_confirm() = 0;
  // virtual bool get_input_cancel() = 0;
};

class KeyboardInputProvider : public InputProvider {
public:
  Vector3 get_input_movement_vector() override;
  Vector2 get_input_look_vector() override;
  bool get_input_jump() override;
  bool get_input_jump_held() override;
  bool get_input_sprint() override;
  bool get_input_blink() override;
  bool get_input_blink_held() override;
  bool get_input_recall() override;
  // bool get_input_attack() override;
  // bool get_input_pause() override;
  // bool get_input_confirm() override;
  // bool get_input_cancel() override;
};

class ControllerInputProvider : public InputProvider {
public:
  Vector3 get_input_movement_vector() override;
  Vector2 get_input_look_vector() override;
  bool get_input_jump() override;
  bool get_input_jump_held() override;
  bool get_input_sprint() override;
  bool get_input_blink() override;
  bool get_input_blink_held() override;
  bool get_input_recall() override;

private:
  static float deadzone(float value, float threshold = 0.15f);
};
