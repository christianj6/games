#include "item.h"
#include "raylib.h"
#include "raymath.h"
#include "systems/runtime/audio.h"

namespace {
constexpr float kPickupRadius = 1.8f;
constexpr float kBeamHeight = 60.0f;
constexpr float kBeamOuterRadius = 0.9f;
constexpr float kBeamInnerRadius = 0.35f;
constexpr Color kBeamOuterColor = {255, 200, 60, 40};
constexpr Color kBeamInnerColor = {255, 220, 90, 110};
constexpr float kBobFreq = 2.5f;
constexpr float kBobAmp = 0.15f;
constexpr float kHoverHeight = 0.6f;
constexpr float kShardRadius = 0.28f;
constexpr float kShardTipRadius = 0.05f;
constexpr float kShardHeight = 0.7f;
constexpr float kShardOrbOffset = 0.45f;
constexpr float kShardOrbRadius = 0.12f;
} // namespace

Item::Item(Vector3 position) {
  current_position_ = position;
  base_y_ = position.y;
}

MovementUpdate Item::update(float dt, Blackboard &blackboard) {
  bob_timer_ += dt;

  // Pickup: 3D distance to the player (keeps shards on pillar tops safe from
  // ground-level grabs through the pillar).
  Vector3 to_player =
      Vector3Subtract(blackboard.current_player_position, current_position_);
  if (Vector3Length(to_player) < kPickupRadius && !collected_) {
    collected_ = true;
    Audio::get().play_at(Sfx::Pickup, blackboard.current_player_position,
                         current_position_, blackboard.listener_right, 50.0f,
                         0.9f);
  }

  return {};
}

void Item::draw() {
  // Beacon beam: tall translucent column visible across the world.
  // Drawn from the shard, so it disappears the moment it is collected.
  Vector3 beam_center = {current_position_.x, base_y_ + kBeamHeight * 0.5f,
                         current_position_.z};
  DrawCylinder(beam_center, kBeamOuterRadius, kBeamOuterRadius, kBeamHeight, 12,
               kBeamOuterColor);
  DrawCylinder(beam_center, kBeamInnerRadius, kBeamInnerRadius, kBeamHeight, 12,
               kBeamInnerColor);

  float bob = sinf(bob_timer_ * kBobFreq) * kBobAmp;
  Vector3 p = current_position_;
  p.y = base_y_ + kHoverHeight + bob;

  DrawCylinder(p, kShardRadius, kShardTipRadius, kShardHeight, 6, GOLD);
  DrawCylinderWires(p, kShardRadius, kShardTipRadius, kShardHeight, 6, ORANGE);
  DrawSphere({p.x, p.y - kShardOrbOffset, p.z}, kShardOrbRadius, ORANGE);
}
