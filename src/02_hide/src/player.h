#pragma once
#include "object.h"

// Player inherits publicly from GameObject
// This means Player is-a GameObject and can be used wherever GameObject is expected
class Player : public GameObject {
  public:
    // Constructor - delegates to GameObject constructor for position initialization
    Player(Vector2 starting_position = {0,0});

    // override keyword explicitly declares we're implementing virtual functions
    // This helps catch errors if base class interface changes
    void update(float dt) override;
    void draw() override;
};
