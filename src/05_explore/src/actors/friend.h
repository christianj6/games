#pragma once
#include "actor.h"

class Friend : public Actor {
public:
  explicit Friend(Renderer *renderer);

  MovementUpdate update(float dt, Blackboard &blackboard) override;
  void draw() override;

private:
  bool friend_marker_ = false;
  bool initialized_ = false;
  void pick_new_target(World *world);
  void lazy_init(World *world);

  float idle_timer_ = 1.0f;
  float heading_deg_ = 0.0f;
};
