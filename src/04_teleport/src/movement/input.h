#pragma once
#include "raylib.h"

class IInputProvider {
public:
  virtual ~IInputProvider() = default;
  virtual Vector3 get_input_movement_vector() = 0;
  virtual Vector2 get_input_look_vector() = 0;
  virtual bool get_input_jump() = 0;
  virtual bool get_input_action() = 0;
  virtual bool get_input_attack() = 0;
};

class KeyboardInputProvider : public IInputProvider {
public:
  KeyboardInputProvider(){};
  Vector3 get_input_movement_vector() override;
  Vector2 get_input_look_vector() override;
  bool get_input_jump() override;
  bool get_input_action() override;
  bool get_input_attack() override;
};

class ControllerInputProvider : public IInputProvider {
public:
  Vector3 get_input_movement_vector() override;
  Vector2 get_input_look_vector() override;
  bool get_input_jump() override;
  bool get_input_action() override;
  bool get_input_attack() override;
};

#ifdef PLATFORM_WEB
// Web pointer-lock look deltas: raylib's web backend keeps only the LAST
// mousemove event per GetMouseDelta() call, so with the ASYNCIFY loop pacing
// (~60-80 Hz) and mice polling at 125-1000 Hz most motion is dropped and
// camera look feels far too slow. JS sums movementX/Y while the canvas holds
// the pointer lock; the main loop reads the sum once per frame and resets it.
void InitWebLookAccumulator(); // one-time JS listener setup
Vector2 ReadWebLookDelta();    // movement accumulated since last reset
void ResetWebLookDelta();      // call once per frame, after reading
#endif
