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
      .position = input_provider->get_input_movement_vector(),
      .camera = input_provider->get_input_look_vector(),
      .jump = input_provider->get_input_jump(),
      .blink = input_provider->get_input_blink(),
      .jump_held = input_provider->get_input_jump_held(),
      .sprint = input_provider->get_input_sprint(),
      .blink_held = input_provider->get_input_blink_held(),
      .recall_held = input_provider->get_input_recall_held(),
      .place_anchor = input_provider->get_input_place_anchor(),
  };
}
