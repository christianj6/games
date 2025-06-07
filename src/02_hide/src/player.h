#pragma once
#include "object.h"

class Player : public GameObject {
  public:
    Player(Vector2 starting_position = {640,400});

    void update(float dt) override;
    void draw() override;
    Vector2 get_position() const { return position; }
};
