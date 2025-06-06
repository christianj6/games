#include "player.h"

// Constructor implementation
// The ": GameObject(starting_position)" syntax calls base class constructor
// This is similar to super().__init__() in Python
Player::Player(Vector2 starting_position) : GameObject(starting_position) {}

// Draw implementation - renders player as blue circle
// position is inherited from GameObject
void Player::draw() { DrawCircle(position.x, position.y, 50, BLUE); }

// Update implementation - currently empty but ready for player movement code
// dt parameter allows for frame-rate independent movement
void Player::update(float dt) {};
