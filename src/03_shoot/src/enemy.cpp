#include "enemy.h"
#include "raymath.h"
#include <math.h>

Enemy::Enemy()
    : radius(0.8f), color(RED), movement_speed(1.3f),
      position(get_random_position(20.0f)) {
  state = EnemyState::PATROL;
  generate_patrol_points(10);
}

Vector3 Enemy::get_random_position(float position_radius) {
  float position_angle =
      GetRandomValue(0, 360) * DEG2RAD; // Random starting angle

  return {position_radius * cosf(position_angle), 3.0f,
          position_radius * sinf(position_angle)};
}
void Enemy::update(float dt) {
  Vector3 target_position;
  switch (state) {
  case EnemyState::PATROL:
    target_position = patrol_points[current_patrol_point_index];
    if (Vector3Distance(position, target_position) < 10.0f) {
      current_patrol_point_index =
          (current_patrol_point_index + 1) % patrol_points.size();
    }
    break;
  case EnemyState::CHASE:
    // TODO
    break;
  case EnemyState::RETURN_TO_PATROL:
    // TODO
    break;
  }

  // Move towards target position
  Vector3 direction = Vector3Subtract(target_position, position);
  direction = Vector3Normalize(direction);

  // Scale by speed and delta time
  direction.x *= movement_speed * dt;
  direction.z *= movement_speed * dt;

  // Update position
  position = Vector3Add(position, direction);
}

void Enemy::update_state() {
  switch (state) {
  case EnemyState::PATROL:
    // TODO
    break;
  case EnemyState::CHASE:
    // TODO
    break;
  case EnemyState::RETURN_TO_PATROL:
    // TODO
    break;
  }
}

void Enemy::generate_patrol_points(int n) {
  patrol_points.clear();
  float patrol_radius = 30.0f;

  for (int i = 0; i < n; i++) {
    patrol_points.push_back(get_random_position(patrol_radius));
  }
}

void Enemy::draw() {
  // Draw the enemy sphere
  DrawSphere(position, radius, color);

  // Draw a shadow circle on the ground
  Vector3 shadow_pos = {position.x, 0.1f, position.z};
  DrawCircle3D(shadow_pos, radius, (Vector3){1, 0, 0}, 90.0f,
               (Color){0, 0, 0, 100});
}
