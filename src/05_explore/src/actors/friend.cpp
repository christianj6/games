#include "friend.h"
#include "raylib.h"
#include "raymath.h"
#include "utils/graphics/renderer.h"
#include "utils/random.h"
#include <cmath>

Friend::Friend(Renderer *renderer) {
  // Staged Kenney-style model; fall back to primitives when unavailable.
  model_ = LoadModel("models/friend.obj");
  if (model_.meshCount > 0) {
    model_loaded_ = true;
    if (renderer != nullptr)
      model_.materials[0].shader = renderer->get_shader();
    BoundingBox box = GetModelBoundingBox(model_);
    float height = box.max.y - box.min.y;
    if (height > 0.01f)
      model_scale_ = 1.7f / height;
    else
      model_scale_ = 1.0f;
  }
}

Friend::~Friend() {
  if (model_loaded_)
    UnloadModel(model_);
}

void Friend::lazy_init(World *world) {
  initialized_ = true;
  current_position = {32.0f, world->get_floor_height(32.0f, 32.0f) + 0.9f, 32.0f};
}

void Friend::pick_new_target(World *world) {
  RandomNumberGenerator<int> rx(4, 60);
  RandomNumberGenerator<int> rz(4, 60);
  // Stay on walkable ground: reject pillar/wall tops.
  for (int attempt = 0; attempt < 10; ++attempt) {
    float x = (float)rx();
    float z = (float)rz();
    if (world->get_floor_height(x, z) <= 2.5f) {
      target_ = {x, 0.0f, z};
      has_target_ = true;
      return;
    }
  }
  has_target_ = false;
}

MovementUpdate Friend::update(float dt, Blackboard &blackboard) {
  World *world = blackboard.world;
  if (world == nullptr)
    return {};
  if (!initialized_)
    lazy_init(world);

  if (!has_target_ && idle_timer_ <= 0.0f)
    pick_new_target(world);

  const float speed = 2.0f;
  if (has_target_) {
    Vector3 delta = {target_.x - current_position.x, 0.0f,
                     target_.z - current_position.z};
    float dist = sqrtf(delta.x * delta.x + delta.z * delta.z);
    if (dist < 0.8f) {
      has_target_ = false;
      RandomNumberGenerator<float> idle(1.0f, 4.0f);
      idle_timer_ = idle();
    } else {
      delta.x /= dist;
      delta.z /= dist;
      heading_deg_ = atan2f(delta.x, delta.z) * RAD2DEG;

      // Axis-separated slide, same convention as the player.
      float step = speed * dt;
      Vector3 full = {current_position.x + delta.x * step, current_position.y,
                      current_position.z + delta.z * step};
      Vector3 probe = {full.x, world->get_floor_height(full.x, full.z) + 2.0f,
                       full.z};
      if (world->position_is_acceptable(probe)) {
        current_position.x = full.x;
        current_position.z = full.z;
      } else {
        has_target_ = false;
      }
      bob_timer_ += dt;
    }
  } else {
    idle_timer_ -= dt;
  }

  // Snap to ground (climbs the 1-block floor naturally).
  current_position.y = world->get_floor_height(current_position.x,
                                               current_position.z) + 0.9f;

  // Talk-range flag for the quest turn-in prompt.
  Vector3 to_player = Vector3Subtract(blackboard.current_player_position,
                                      current_position);
  if (Vector3Length(to_player) < 3.0f)
    blackboard.friend_nearby = true;

  friend_marker_ = (blackboard.quest.state == QuestState::TURN_IN);

  return {};
}

void Friend::draw() {
  if (model_loaded_) {
    DrawModelEx(model_, current_position, {0.0f, 1.0f, 0.0f}, heading_deg_,
                {model_scale_, model_scale_, model_scale_}, WHITE);
  } else {
    DrawCylinder(current_position, 0.35f, 0.35f, 1.1f, 10, SKYBLUE);
    DrawSphere({current_position.x, current_position.y + 0.85f,
                current_position.z}, 0.3f, BEIGE);
  }

  // Quest marker: request pending, exclamation above head.
  if (friend_marker_) {
    Vector3 p = current_position;
    p.y += 2.0f;
    DrawSphere(p, 0.12f, YELLOW);
    DrawCylinder({p.x, p.y + 0.3f, p.z}, 0.05f, 0.05f, 0.35f, 6, YELLOW);
  }
}
