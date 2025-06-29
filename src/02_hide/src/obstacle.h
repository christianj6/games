#pragma once
#include "object.h"
#include <vector>

// public inheritance means we take everything from parent 'as-is'
class Obstacle : public GameObject {
public:
    Obstacle(Vector2 starting_position);
    void draw() override;
    void update(float dt, bool allow_movement = false) override;
};

std::vector<Obstacle> get_world_obstacles(int n);
