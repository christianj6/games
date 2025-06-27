#include "enemy.h"
#include "fmt/core.h"
#include "raymath.h"
#include "utils/random.h"
#include <fmt/base.h>

Enemy::Enemy()
    : radius(1.0f), color(DARKGRAY), current_state(EnemyState::PATROLLING) {
  position = get_random_world_position(2);
}

bool Enemy::update(float dt, Vector3 &current_player_position) {
  bool is_killable;
  switch (current_state) {
  case (EnemyState::CHASING):
    // TODO
    break;
  case (EnemyState::SEARCHING):
    // TODO
    break;
  case (EnemyState::DEAD):
    break;
  case (EnemyState::PATROLLING):
    float distance_to_player =
        Vector3Length(Vector3Subtract(current_player_position, position));
    if (distance_to_player <= 3.5f) {
      color = RED;
      is_killable = true;
    } else {
      color = DARKGRAY;
    }
    break;
  }
  return is_killable;
}

void Enemy::disable() { current_state = EnemyState::DEAD; }

void Enemy::draw() {
  if (!(current_state == EnemyState::DEAD)) {
    DrawSphere(position, radius, color);
  }
}
