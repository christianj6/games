#pragma once
#include "object.h"

class Player : public GameObject {
  public:
    Player(Vector2 starting_position = {0,0});

    void update(float dt) override;
    void draw() override;
    Vector2 get_position() const { return position; }
};
