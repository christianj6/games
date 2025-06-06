#include "player.h"

// constructor calls base class constructor like python super().__init__()
Player::Player(Vector2 starting_position) : GameObject(starting_position) {}

void Player::draw() { DrawCircle(position.x, position.y, 50, BLUE); }
void Player::update(float dt) {};
