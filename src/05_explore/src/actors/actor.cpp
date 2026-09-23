#include "actor.h"

#include "utils/graphics/renderer.h"

#include <cmath>

namespace {
constexpr float kMinModelHeight =
    0.01f; // below this the bounding box is degenerate
} // namespace

Actor::~Actor() { unload_actor_model(); }

MovementUpdate Actor::get_update(float, Blackboard &) {
  return movement_controller_->tick();
}

// Staged Kenney-style model; fall back to primitives when unavailable.
void Actor::load_actor_model(const char *path, float target_height,
                             Renderer *renderer) {
  model_ = LoadModel(path);
  if (model_.meshCount > 0) {
    model_loaded_ = true;
    if (renderer != nullptr)
      model_.materials[0].shader = renderer->get_shader();
    BoundingBox box = GetModelBoundingBox(model_);
    float height = box.max.y - box.min.y;
    model_scale_ = height > kMinModelHeight ? target_height / height : 1.0f;
  }
}

void Actor::unload_actor_model() {
  if (model_loaded_)
    UnloadModel(model_);
}

// Stay on walkable ground: reject pillar/wall tops.
bool Actor::try_place_target(World *world, Vector3 candidate) {
  if (world->get_floor_height(candidate.x, candidate.z) > kWalkableFloorMax)
    return false;
  target_ = candidate;
  has_target_ = true;
  return true;
}

// Horizontal distance to the waypoint; clears it once arrived.
bool Actor::target_reached(float radius) {
  Vector3 d = {target_.x - current_position_.x, 0.0f,
               target_.z - current_position_.z};
  if (sqrtf(d.x * d.x + d.z * d.z) >= radius)
    return false;
  has_target_ = false;
  return true;
}
