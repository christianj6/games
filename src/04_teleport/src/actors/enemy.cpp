#include "enemy.h"
#include "fmt/core.h"
#include "utils/random.h"
#include <fmt/base.h>

Enemy::Enemy() : radius(1.0f), color(RED) {
  position = get_random_world_position(2);
}

void Enemy::update(float dt, Vector3 &current_player_position) {
  // TODO
}

void Enemy::draw() {
  DrawSphere(position, radius, color);
}
