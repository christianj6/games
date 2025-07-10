#include "actor.h"
#include "systems/movement/controller.h"
#include <memory>

void Actor::set_world(std::shared_ptr<World> world) {
  // TODO
}

void Actor::get_nearest_available_position(Vector3 target_position) {
  // TODO
}

MovementUpdate Actor::get_update(float, Blackboard &) {
  return movement_controller->tick();
}
