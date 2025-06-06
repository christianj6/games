#include "player.h"

// call base class constructor, similar to super().__init__() in python
Player::Player(Vector2 starting_position) : GameObject(starting_position) {}

void Player::draw() { DrawCircle(position.x, position.y, 50, BLUE); }
void Player::update(float dt) {};
