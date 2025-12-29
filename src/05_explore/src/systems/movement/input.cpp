#include "input.h"
#include "raylib.h"

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

bool KeyboardInputProvider::get_input_jump() { 
  return IsKeyPressed(KEY_SPACE);
}

bool KeyboardInputProvider::get_input_blink() { return false; }
