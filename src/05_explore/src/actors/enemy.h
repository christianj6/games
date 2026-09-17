#pragma once
#include "actor.h"
#include <behaviortree_cpp/bt_factory.h>

class Enemy : public Actor {
public:
  explicit Enemy(Renderer *renderer, Vector3 guard_post);
  ~Enemy() override;

  bool is_dead() const override { return dead_; }
  bool is_incapacitated() const { return incapacitated_; }
  Vector3 get_position() const { return current_position; }
  void incapacitate();
  void revive();
  void set_revive_target(Enemy *ally);
  MovementUpdate update(float dt, Blackboard &blackboard) override;
  void draw() override;

private:
  void build_tree();
  void lazy_init(World *world);
  void pick_patrol_target(World *world);
  bool can_see_player(Blackboard &blackboard);
  void draw_vision_cone();
  void move_toward(World *world, Vector3 target, float speed, float dt);

  // Per-frame inputs the BT conditions/actions read and behaviors write.
  struct FrameContext {
    Blackboard *blackboard = nullptr;
    World *world = nullptr;
    float dt = 0.0f;
    bool sees_player = false;
    float player_distance = 0.0f;
  };
  FrameContext ctx_;
  BT::BehaviorTreeFactory factory_;
  BT::Tree tree_;

  bool initialized_ = false;
  bool dead_ = false;
  Vector3 guard_post_ = {0, 0, 0};
  Vector3 target_ = {0, 0, 0};
  bool has_target_ = false;
  float leash_radius_ = 60.0f; // hard chase limit from guard_post_
  float idle_timer_ = 0.0f;
  float alert_ = 0.0f; // 0 = calm, 1 = fully alerted
  Vector3 last_seen_ = {0, 0, 0};
  float shoot_cooldown_ = 0.0f;
  float tracer_timer_ = 0.0f;
  Vector3 tracer_from_ = {0, 0, 0};
  Vector3 tracer_to_ = {0, 0, 0};
  bool incapacitated_ = false;
  float revive_timer_ = 0.0f;
  Enemy *revive_target_ = nullptr;
  bool alert_announced_ = false;
  Vector3 last_position_ = {0, 0, 0};
  float stuck_timer_ = 0.0f;
  float unstuck_timer_ = 0.0f;
  Vector3 unstuck_dir_ = {0, 0, 0};
  float heading_deg_ = 0.0f;
  Model model_{};
  Sound buzz_{};
  bool buzz_ready_ = false;
  bool model_loaded_ = false;
  float model_scale_ = 1.0f;
};
