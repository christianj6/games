#pragma once
#include "raylib.h"

class InputProvider {
public:
  InputProvider() = default;
  virtual ~InputProvider() = default;
  virtual Vector3 get_input_movement_vector() = 0;
  virtual Vector2 get_input_look_vector() = 0;
  virtual bool get_input_jump() = 0;
  virtual bool get_input_jump_held() = 0;
  virtual bool get_input_sprint() = 0;
  virtual bool get_input_blink() = 0;
  virtual bool get_input_blink_held() = 0;
  virtual bool get_input_recall_held() = 0;
  virtual bool get_input_place_anchor() = 0;
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
  bool get_input_recall_held() override;
  bool get_input_place_anchor() override;
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
  bool get_input_recall_held() override;
  bool get_input_place_anchor() override;
  // LB press edge, kept off the shared interface: only the gamepad anchor
  // chord needs it; the keyboard path derives its edge in Player.
  bool get_input_recall();
};

#ifdef PLATFORM_WEB
// Web pointer-lock look deltas: raylib's web backend keeps only the LAST
// mousemove event per GetMouseDelta() call, so with the ASYNCIFY loop pacing
// (~60-80 Hz) and mice polling at 125-1000 Hz most motion is dropped and
// camera look feels far too slow. JS sums movementX/Y while the canvas holds
// the pointer lock; the main loop reads the sum once per frame and resets it.
void init_web_look_accumulator(); // one-time JS listener setup
Vector2 read_web_look_delta();    // movement accumulated since last reset
void reset_web_look_delta();      // call once per frame, after reading
#endif
