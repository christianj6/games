#include "input.h"
#include <cmath>

Vector2 KeyboardInputProvider::get_input_look_vector() {
  return GetMouseDelta();
}

bool KeyboardInputProvider::get_input_jump() { return IsKeyPressed(KEY_SPACE); }

bool KeyboardInputProvider::get_input_action() {
  return IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

Vector2 ControllerInputProvider::get_input_look_vector() {
  Vector2 lookDir = {0};

  if (IsGamepadAvailable(0)) {
    float rightX = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X);
    float rightY = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y);

    const float deadzone = 0.2f;
    if (fabs(rightX) > deadzone) {
      lookDir.x = rightX * 10.0f; // Scale up to match mouse sensitivity
    }
    if (fabs(rightY) > deadzone) {
      lookDir.y = rightY * 10.0f;
    }
  }

  return lookDir;
}

bool ControllerInputProvider::get_input_jump() {
  return IsGamepadButtonPressed(
      0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN); // A/Cross button
}

bool ControllerInputProvider::get_input_action() {
  return IsGamepadButtonPressed(0,
                                GAMEPAD_BUTTON_RIGHT_TRIGGER_1); // RB/R1 button
}

Vector3 KeyboardInputProvider::get_input_movement_vector() {
  Vector3 inputDir = {0};

  // Forward/Backward (local z-axis)
  if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
    inputDir.z += 1.0f;
  }
  if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) {
    inputDir.z -= 1.0f;
  }

  // Left/Right (local x-axis)
  if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
    inputDir.x += 1.0f;
  }
  if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
    inputDir.x -= 1.0f;
  }

  return inputDir;
}

Vector3 ControllerInputProvider::get_input_movement_vector() {
  Vector3 inputDir = {0};

  // Check if a gamepad is available
  if (IsGamepadAvailable(0)) { // First gamepad
    // Get left stick values and apply deadzone
    float leftX = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
    float leftY = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);

    // Apply deadzone of 0.2 to prevent drift
    const float deadzone = 0.2f;
    if (fabs(leftX) > deadzone) {
      inputDir.x = leftX;
    }
    if (fabs(leftY) > deadzone) {
      inputDir.z = -leftY; // Inverted because forward is negative on Y axis
    }
  }

  return inputDir;
}
