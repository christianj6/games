#pragma once
#include "actor.h"

class Friend : public Actor {
public:
  explicit Friend(Renderer *renderer);
  ~Friend() override;

  MovementUpdate update(float dt, Blackboard &blackboard) override;
  void draw() override;

private:
  bool friend_marker_ = false;
  bool initialized_ = false;
  void pick_new_target(World *world);
  void lazy_init(World *world);

  Model model_{};
  bool model_loaded_ = false;
  float model_scale_ = 1.0f;
  Vector3 target_ = {32, 0, 32};
  bool has_target_ = false;
  float idle_timer_ = 1.0f;
  float heading_deg_ = 0.0f;
  float bob_timer_ = 0.0f;
};
