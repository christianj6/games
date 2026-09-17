#include "input.h"

#ifdef PLATFORM_WEB
#include <emscripten.h>
#endif
#include "raylib.h"
#include <cmath>

Vector2 KeyboardInputProvider::get_input_look_vector() {
#ifdef PLATFORM_WEB
  return ReadWebLookDelta();
#else
  return GetMouseDelta();
#endif
}

bool KeyboardInputProvider::get_input_jump() { return IsKeyPressed(KEY_SPACE); }

bool KeyboardInputProvider::get_input_attack() {
  return IsMouseButtonDown(MOUSE_BUTTON_LEFT);
}

bool KeyboardInputProvider::get_input_action() {
  return IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
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

bool ControllerInputProvider::get_input_attack() {
  return IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_1); // RB
}

bool ControllerInputProvider::get_input_action() {
  return IsGamepadButtonDown(0,
                             GAMEPAD_BUTTON_LEFT_TRIGGER_2); // LT
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

#ifdef PLATFORM_WEB
void InitWebLookAccumulator() {
  EM_ASM({
    if (Module._lookInit)
      return;
    Module._lookInit = true;
    Module._lookDX = 0;
    Module._lookDY = 0;
    document.addEventListener(
        'mousemove', function(e) {
          // Sum only while the canvas holds the pointer lock; unlocked menus
          // use raylib's CSS-mapped cursor position instead.
          if (document.pointerLockElement === Module.canvas) {
            Module._lookDX += e.movementX;
            Module._lookDY += e.movementY;
          }
        });
  });
}

Vector2 ReadWebLookDelta() {
  Vector2 delta;
  delta.x = (float)EM_ASM_DOUBLE({ return Module._lookDX || 0; });
  delta.y = (float)EM_ASM_DOUBLE({ return Module._lookDY || 0; });
  return delta;
}

void ResetWebLookDelta() {
  EM_ASM({
    if (Module._lookInit) {
      Module._lookDX = 0;
      Module._lookDY = 0;
    }
  });
}
#endif
