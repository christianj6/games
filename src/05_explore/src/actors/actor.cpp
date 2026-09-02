#include "actor.h"

MovementUpdate Actor::get_update(float, Blackboard &) {
  return movement_controller->tick();
}
