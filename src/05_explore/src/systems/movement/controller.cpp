#include "controller.h"

MovementUpdate UserMovementController::tick() {
  return {
      input_provider->get_input_movement_vector(),
      input_provider->get_input_look_vector(),
      input_provider->get_input_jump(),
      input_provider->get_input_blink(),
  };
}
