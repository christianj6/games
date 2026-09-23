#include "input.h"

#ifdef PLATFORM_WEB
#include <emscripten.h>
#endif
#include "raylib.h"
#include <cmath>

namespace {
// Stick and look tuning.
constexpr float kStickDeadzone = 0.15f;
constexpr float kLookStickScale = 30.0f;

// Keyboard and mouse bindings — the whole control scheme in one place.
constexpr int kKeyForward = KEY_W;
constexpr int kKeyBack = KEY_S;
constexpr int kKeyRight = KEY_D;
constexpr int kKeyLeft = KEY_A;
constexpr int kKeyJump = KEY_SPACE;
constexpr int kKeySprint = KEY_LEFT_SHIFT;
constexpr int kKeyRecall = KEY_Q;
constexpr int kKeyAnchor = KEY_F;
constexpr int kBtnBlink = MOUSE_BUTTON_RIGHT;

float deadzone(float value) {
  return fabsf(value) > kStickDeadzone ? value : 0.0f;
}
} // namespace

Vector3 KeyboardInputProvider::get_input_movement_vector() {
  Vector3 movement = {0};

  if (IsKeyDown(kKeyForward)) {
    movement.z += 1.0f;
  }
  if (IsKeyDown(kKeyBack)) {
    movement.z -= 1.0f;
  }

  if (IsKeyDown(kKeyRight)) {
    movement.x += 1.0f;
  }
  if (IsKeyDown(kKeyLeft)) {
    movement.x -= 1.0f;
  }

  return movement;
}

Vector2 KeyboardInputProvider::get_input_look_vector() {
#ifdef PLATFORM_WEB
  return read_web_look_delta();
#else
  return GetMouseDelta();
#endif
}

bool KeyboardInputProvider::get_input_jump() { return IsKeyPressed(kKeyJump); }

bool KeyboardInputProvider::get_input_jump_held() {
  return IsKeyDown(kKeyJump);
}

bool KeyboardInputProvider::get_input_sprint() { return IsKeyDown(kKeySprint); }

bool KeyboardInputProvider::get_input_blink() {
  return IsMouseButtonPressed(kBtnBlink);
}

bool KeyboardInputProvider::get_input_blink_held() {
  return IsMouseButtonDown(kBtnBlink);
}

bool KeyboardInputProvider::get_input_recall_held() {
  return IsKeyDown(kKeyRecall);
}

bool KeyboardInputProvider::get_input_place_anchor() {
  return IsKeyPressed(kKeyAnchor);
}

Vector3 ControllerInputProvider::get_input_movement_vector() {
  float x = deadzone(GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X));
  float z = deadzone(GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y));
  return {x, 0.0f, -z};
}

Vector2 ControllerInputProvider::get_input_look_vector() {
  float x = deadzone(GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X));
  float y = deadzone(GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y));
  return {x * kLookStickScale, y * kLookStickScale};
}

bool ControllerInputProvider::get_input_jump() {
  return IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
}

bool ControllerInputProvider::get_input_jump_held() {
  return IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
}

bool ControllerInputProvider::get_input_sprint() {
  return IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_THUMB);
}

bool ControllerInputProvider::get_input_blink() {
  return IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_1);
}

bool ControllerInputProvider::get_input_blink_held() {
  return IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_1);
}

bool ControllerInputProvider::get_input_recall() {
  return IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_TRIGGER_1);
}

bool ControllerInputProvider::get_input_recall_held() {
  return IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_TRIGGER_1);
}

bool ControllerInputProvider::get_input_place_anchor() {
  // Anchor chord: one trigger pressed while the other is already held
  bool lb_pressed = get_input_recall();
  bool rb_pressed = get_input_blink();
  bool lb_held = get_input_recall_held();
  bool rb_held = get_input_blink_held();
  return (lb_pressed && rb_held) || (lb_held && rb_pressed);
}

#ifdef PLATFORM_WEB
// clang-format off
// The EM_ASM bodies below are JavaScript, not C++. clang-format lexes them as
// C++ tokens and rewrites `===` into `== =`, which is a JS syntax error the
// desktop build never compiles and so never catches. Leave them unformatted.
void init_web_look_accumulator() {
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

Vector2 read_web_look_delta() {
  Vector2 delta;
  delta.x = (float)EM_ASM_DOUBLE({ return Module._lookDX || 0; });
  delta.y = (float)EM_ASM_DOUBLE({ return Module._lookDY || 0; });
  return delta;
}

void reset_web_look_delta() {
  EM_ASM({
    if (Module._lookInit) {
      Module._lookDX = 0;
      Module._lookDY = 0;
    }
  });
}
// clang-format on
#endif
