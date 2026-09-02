#pragma once
#include "actor.h"

class Item : public Actor {
public:
  Item(Vector3 position);

  bool is_collected() const { return collected_; }

  MovementUpdate update(float dt, Blackboard &blackboard) override;
  void draw() override;

private:
  bool collected_ = false;
  float base_y_ = 0.0f;
  float bob_timer_ = 0.0f;
};
