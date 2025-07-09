#include "actor.h"
#include "systems/movement/controller.h"
#include <memory>

void Actor::set_world(std::shared_ptr<World> world) {
  // TODO
}

void Actor::get_nearest_available_position(Vector3 target_position) {
  // TODO
}

Vector3 add_vectors(Vector3 v1, Vector3 v2) {
  v1.x += v2.x;
  v1.z += v2.z;

  return v1;
}

MovementUpdate Actor::update(float, Blackboard &) {
  const MovementUpdate update = movement_controller->tick();
  Vector3 candidate_position = add_vectors(current_position, update.position);
  if (world->position_is_acceptable(candidate_position)) {
    current_position = candidate_position;
  }

  // TODO

  return update;
}
