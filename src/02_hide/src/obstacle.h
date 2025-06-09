#pragma once
#include "object.h"
#include <vector>

class Obstacle : public GameObject {
public:
    Obstacle(Vector2 starting_position);
    void draw() override;
    void update(float dt) override;
};

std::vector<Obstacle> get_world_obstacles(int n);
