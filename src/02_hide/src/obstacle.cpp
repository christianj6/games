#include "obstacle.h"

Obstacle::Obstacle(Vector2 starting_position) : GameObject(starting_position) {}

void Obstacle::draw() { DrawCircle(position.x, position.y, 15, GRAY); }

void Obstacle::update(float dt) {}
