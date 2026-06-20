#include "controller.h"

MovementUpdate UserMovementController::tick() {
  bool gamepad_available = IsGamepadAvailable(0);
  if (gamepad_available && !using_gamepad_) {
    input_provider = std::make_unique<ControllerInputProvider>();
    using_gamepad_ = true;
  } else if (!gamepad_available && using_gamepad_) {
    input_provider = std::make_unique<KeyboardInputProvider>();
    using_gamepad_ = false;
  }

  return {
      input_provider->get_input_movement_vector(),
      input_provider->get_input_look_vector(),
      input_provider->get_input_jump(),
      input_provider->get_input_blink(),
      input_provider->get_input_jump_held(),
  };
}
