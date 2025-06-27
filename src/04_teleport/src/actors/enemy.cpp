#include "enemy.h"
#include "fmt/core.h"
#include "raymath.h"
#include "utils/random.h"
#include <fmt/base.h>

Enemy::Enemy()
    : radius(1.0f), color(DARKGRAY), current_state(EnemyState::PATROLLING),
      horizontal_rotation(0.0f), vertical_rotation(0.0f),
      horizontal_fov(PI / 2.0f), vertical_fov(PI / 3.0f), vision_range(20.0f),
      forward_vector({1.0f, 0.0f, 0.0f}) {
  position = get_random_world_position(2);
}

bool Enemy::is_in_vision_cone(const Vector3 &target) const {
  Vector3 to_target = Vector3Subtract(target, position);
  float distance = Vector3Length(to_target);

  if (distance > vision_range)
    return false;

  // Normalize the vector to target
  to_target = Vector3Scale(to_target, 1.0f / distance);

  // Check horizontal angle
  float horizontal_dot =
      to_target.x * forward_vector.x + to_target.z * forward_vector.z;
  float horizontal_angle = acosf(horizontal_dot);
  if (fabs(horizontal_angle) > horizontal_fov / 2.0f)
    return false;

  // Check vertical angle
  float vertical_angle = asinf(to_target.y);
  if (fabs(vertical_angle) > vertical_fov / 2.0f)
    return false;

  return true;
}

bool Enemy::update(float dt, Vector3 &current_player_position) {
  bool is_killable = false;
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

    can_see_player = is_in_vision_cone(current_player_position);

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

void Enemy::draw_vision_cone() const {
  if (current_state == EnemyState::DEAD)
    return;

  const int horizontal_segments = 8;
  const int vertical_segments = 6;
  const float h_half_angle = horizontal_fov / 2.0f;
  const float v_half_angle = vertical_fov / 2.0f;

  // Draw horizontal rings at different vertical angles
  for (int v = 0; v <= vertical_segments; v++) {
    float v_angle = -v_half_angle + (vertical_fov * v / vertical_segments);

    for (int h = 0; h <= horizontal_segments; h++) {
      float h_angle =
          -h_half_angle + (horizontal_fov * h / horizontal_segments);

      // Calculate point on the cone surface
      float cos_h = cosf(h_angle);
      float sin_h = sinf(h_angle);
      float cos_v = cosf(v_angle);
      float sin_v = sinf(v_angle);

      Vector3 direction = {cos_v * cos_h, sin_v, cos_v * sin_h};

      Vector3 end_point =
          Vector3Add(position, Vector3Scale(direction, vision_range));

      // Draw lines to create a wireframe effect
      Color cone_color = can_see_player ? RED : YELLOW;
      if (h < horizontal_segments) {
        // Calculate next horizontal point
        float next_h_angle =
            -h_half_angle + (horizontal_fov * (h + 1) / horizontal_segments);
        Vector3 next_direction = {cos_v * cosf(next_h_angle), sin_v,
                                  cos_v * sinf(next_h_angle)};
        Vector3 next_point =
            Vector3Add(position, Vector3Scale(next_direction, vision_range));
        DrawLine3D(end_point, next_point, ColorAlpha(cone_color, 0.3f));
      }

      if (v < vertical_segments) {
        // Calculate next vertical point
        float next_v_angle =
            -v_half_angle + (vertical_fov * (v + 1) / vertical_segments);
        Vector3 next_direction = {cosf(next_v_angle) * cos_h,
                                  sinf(next_v_angle),
                                  cosf(next_v_angle) * sin_h};
        Vector3 next_point =
            Vector3Add(position, Vector3Scale(next_direction, vision_range));
        DrawLine3D(end_point, next_point, ColorAlpha(cone_color, 0.3f));
      }

      // Draw line from origin to point
      if (h % 2 == 0 && v % 2 == 0) {
        DrawLine3D(position, end_point, ColorAlpha(cone_color, 0.15f));
      }
    }
  }
}

void Enemy::draw() {
  if (!(current_state == EnemyState::DEAD)) {
    DrawSphere(position, radius, color);
    draw_vision_cone();
  }
}
