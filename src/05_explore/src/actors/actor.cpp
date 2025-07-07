#include "actor.h"
#include <memory>

void Actor::set_world(std::shared_ptr<World> world) {
  // TODO
}

void Actor::get_nearest_available_position(Vector3 target_position) {
  // TODO
}

Vector3 Actor::update(float, Blackboard&) {
  const MovementUpdate candidate = movement_controller->tick();
  if (world->position_is_acceptable(candidate.position)) {
     current_position = candidate.position;
  }

  // TODO

  return current_position;
}
