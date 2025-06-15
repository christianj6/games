#include "enemy.h"
#include <math.h>

Enemy::Enemy() {
  patrol_radius = 15.0f;
  patrol_speed = 0.3f;
  patrol_angle = GetRandomValue(0, 360) * DEG2RAD; // Random starting angle

  // Initialize position
  position = {patrol_radius * cosf(patrol_angle), 3.0f,
              patrol_radius * sinf(patrol_angle)};

  sphere_radius = 0.8f;
  color = RED;
}

void Enemy::update(float dt) {
  // Update patrol angle
  patrol_angle += patrol_speed * dt;
  if (patrol_angle > 2 * PI) {
    patrol_angle -= 2 * PI;
  }

  // Update position based on patrol circle - simpler version
  position.x = patrol_radius * cosf(patrol_angle);
  position.z = patrol_radius * sinf(patrol_angle);
}

void Enemy::draw() {
  // Draw the enemy sphere
  DrawSphere(position, sphere_radius, color);

  // Draw a shadow circle on the ground
  Vector3 shadow_pos = {position.x, 0.1f, position.z};
  DrawCircle3D(shadow_pos, sphere_radius, (Vector3){1, 0, 0}, 90.0f,
               (Color){0, 0, 0, 100});
}
