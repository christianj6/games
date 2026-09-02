#include "item.h"
#include "raylib.h"
#include "systems/runtime/audio.h"
#include "raymath.h"

Item::Item(Vector3 position) { current_position = position; base_y_ = position.y; }

MovementUpdate Item::update(float dt, Blackboard &blackboard) {
  spin_ += dt * 2.0f;
  bob_timer_ += dt;

  // Pickup: 3D distance to the player (keeps shards on pillar tops safe from
  // ground-level grabs through the pillar).
  Vector3 to_player = Vector3Subtract(blackboard.current_player_position,
                                      current_position);
  if (Vector3Length(to_player) < 1.8f && !collected_) {
    collected_ = true;
    Audio::get().play_at(Sfx::Pickup, blackboard.current_player_position,
                         current_position, blackboard.listener_right, 50.0f,
                         0.9f);
  }

  return {};
}

void Item::draw() {
  // Beacon beam: tall translucent column visible across the world.
  // Drawn from the shard, so it disappears the moment it is collected.
  const float beam_height = 60.0f;
  Vector3 beam_center = {current_position.x, base_y_ + beam_height * 0.5f,
                         current_position.z};
  DrawCylinder(beam_center, 0.9f, 0.9f, beam_height, 12, {255, 200, 60, 40});
  DrawCylinder(beam_center, 0.35f, 0.35f, beam_height, 12, {255, 220, 90, 110});

  float bob = sinf(bob_timer_ * 2.5f) * 0.15f;
  Vector3 p = current_position;
  p.y = base_y_ + 0.6f + bob;

  DrawCylinder(p, 0.28f, 0.05f, 0.7f, 6, GOLD);
  DrawCylinderWires(p, 0.28f, 0.05f, 0.7f, 6, ORANGE);
  DrawSphere({p.x, p.y - 0.45f, p.z}, 0.12f, ORANGE);
}
