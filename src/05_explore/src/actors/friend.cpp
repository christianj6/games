#include "friend.h"
#include "raylib.h"
#include "raymath.h"
#include "utils/random.h"
#include <cmath>

namespace {
constexpr float kFriendSpeed = 2.0f;
constexpr float kSpawnPosition = 32.0f;
constexpr int kWanderMin = 4;
constexpr int kWanderMax = 60;
constexpr float kArriveRadius = 0.8f;
constexpr float kIdleMin = 1.0f;
constexpr float kIdleMax = 4.0f;
constexpr float kTalkRange = 3.0f;
constexpr float kModelTargetHeight = 1.7f;
} // namespace

Friend::Friend(Renderer *renderer) {
  load_actor_model("models/friend.obj", kModelTargetHeight, renderer);
}

void Friend::lazy_init(World *world) {
  initialized_ = true;
  current_position_ = {kSpawnPosition,
                       world->get_floor_height(kSpawnPosition, kSpawnPosition) +
                           kActorHalfHeight,
                       kSpawnPosition};
}

void Friend::pick_new_target(World *world) {
  RandomNumberGenerator<int> rx(kWanderMin, kWanderMax);
  RandomNumberGenerator<int> rz(kWanderMin, kWanderMax);
  for (int attempt = 0; attempt < 10; ++attempt) {
    float x = (float)rx();
    float z = (float)rz();
    if (try_place_target(world, {x, 0.0f, z}))
      return;
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

  if (has_target_) {
    if (target_reached(kArriveRadius)) {
      RandomNumberGenerator<float> idle(kIdleMin, kIdleMax);
      idle_timer_ = idle();
    } else {
      Vector3 delta = {target_.x - current_position_.x, 0.0f,
                       target_.z - current_position_.z};
      float dist = sqrtf(delta.x * delta.x + delta.z * delta.z);
      delta.x /= dist;
      delta.z /= dist;
      heading_deg_ = atan2f(delta.x, delta.z) * RAD2DEG;

      // Single full-step probe; abandon the target if blocked.
      float step = kFriendSpeed * dt;
      Vector3 full = {current_position_.x + delta.x * step, current_position_.y,
                      current_position_.z + delta.z * step};
      Vector3 probe = {full.x, world->get_floor_height(full.x, full.z) + 2.0f,
                       full.z};
      if (world->position_is_acceptable(probe)) {
        current_position_.x = full.x;
        current_position_.z = full.z;
      } else {
        has_target_ = false;
      }
    }
  } else {
    idle_timer_ -= dt;
  }

  // Snap to ground (climbs the 1-block floor naturally).
  current_position_.y =
      world->get_floor_height(current_position_.x, current_position_.z) +
      kActorHalfHeight;

  // Talk-range flag for the quest turn-in prompt.
  Vector3 to_player =
      Vector3Subtract(blackboard.current_player_position, current_position_);
  if (Vector3Length(to_player) < kTalkRange)
    blackboard.friend_nearby = true;

  friend_marker_ = (blackboard.quest.state == QuestState::TURN_IN);

  return {};
}

void Friend::draw() {
  if (model_loaded_) {
    DrawModelEx(model_, current_position_, {0.0f, 1.0f, 0.0f}, heading_deg_,
                {model_scale_, model_scale_, model_scale_}, WHITE);
  } else {
    DrawCylinder(current_position_, 0.35f, 0.35f, 1.1f, 10, SKYBLUE);
    DrawSphere(
        {current_position_.x, current_position_.y + 0.85f, current_position_.z},
        0.3f, BEIGE);
  }

  // Quest marker: request pending, exclamation above head.
  if (friend_marker_) {
    Vector3 p = current_position_;
    p.y += 2.0f;
    DrawSphere(p, 0.12f, YELLOW);
    DrawCylinder({p.x, p.y + 0.3f, p.z}, 0.05f, 0.05f, 0.35f, 6, YELLOW);
  }
}
