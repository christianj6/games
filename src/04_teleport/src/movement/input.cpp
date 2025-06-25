#include "input.h"

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
  // TODO
}
