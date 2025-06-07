#pragma once
#include "object.h"

class Obstacle : public GameObject {
public:
    Obstacle(Vector2 starting_position);
    void draw() override;
    void update(float dt) override;
};
