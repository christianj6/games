#include "input.h"
#include "raylib.h"
#include <cmath>

Vector3 KeyboardInputProvider::get_input_movement_vector() {
  Vector3 movement = {0};

  if (IsKeyDown(KEY_W)) {
    movement.z += 1.0f;
  }
  if (IsKeyDown(KEY_S)) {
    movement.z -= 1.0f;
  }

  if (IsKeyDown(KEY_D)) {
    movement.x += 1.0f;
  }
  if (IsKeyDown(KEY_A)) {
    movement.x -= 1.0f;
  }

  return movement;
}

Vector2 KeyboardInputProvider::get_input_look_vector() {
  return GetMouseDelta();
}

bool KeyboardInputProvider::get_input_jump() { return IsKeyPressed(KEY_SPACE); }

bool KeyboardInputProvider::get_input_jump_held() { return IsKeyDown(KEY_SPACE); }

bool KeyboardInputProvider::get_input_blink() { return false; }

float ControllerInputProvider::deadzone(float value, float threshold) {
  return fabsf(value) > threshold ? value : 0.0f;
}

Vector3 ControllerInputProvider::get_input_movement_vector() {
  float x = deadzone(GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X));
  float z = deadzone(GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y));
  return {x, 0.0f, -z};
}

Vector2 ControllerInputProvider::get_input_look_vector() {
  const float look_scale = 30.0f;
  float x = deadzone(GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X));
  float y = deadzone(GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y));
  return {x * look_scale, y * look_scale};
}

bool ControllerInputProvider::get_input_jump() {
  return IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
}

bool ControllerInputProvider::get_input_jump_held() {
  return IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
}

bool ControllerInputProvider::get_input_blink() {
  return IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_TRIGGER_1);
}
