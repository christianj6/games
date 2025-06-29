#pragma once
#include "object.h"

class Player : public GameObject {
  public:
    Player(Vector2 starting_position = {0,0});

    void update(float dt, bool allow_movement = true) override;
    void draw() override;
    // const signals that we won't modify the instance
    Vector2 get_position() const { return position; }
    void set_position(Vector2 pos) { position = pos; }
};
