#include "controller.h"
#include "movement/input.h"
#include "raylib.h"
#include <memory>

PlayerMovementController::PlayerMovementController(
    std::unique_ptr<IInputProvider> input_provider)
    : input_provider(std::move(input_provider)) {}

Vector3 PlayerMovementController::update_movement(float dt, Vector3 forward,
                                                  Vector3 right) {
  const float speed = 4.5f;

  Vector3 input_direction_vector = input_provider->get_input_movement_vector();
  Vector3 movement = {(right.x * input_direction_vector.x +
                       forward.x * input_direction_vector.z) *
                          speed * dt,
                      0.0f, // Assuming no vertical movement for now
                      (right.z * input_direction_vector.x +
                       forward.z * input_direction_vector.z) *
                          speed * dt};
  return movement;
}

Vector2 PlayerMovementController::update_camera() {
  return input_provider->get_input_look_vector();
}
