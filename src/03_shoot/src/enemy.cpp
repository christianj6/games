#include "enemy.h"
#include "raylib.h"
#include "raymath.h"
#include <math.h>

const float PATROL_SPEED = 2.0f; // Original speed
const float CHASE_SPEED = 4.5f;  // Faster speed when chasing

Enemy::Enemy()
    : radius(0.8f), color(RED), movement_speed(PATROL_SPEED),
      position(get_random_position(20.0f)),
      vision_angle(PI / 3.0f), // 60 degrees
      vision_range(20.0f), facing_direction({1.0f, 0.0f, 0.0f}),
      chase_cooldown_timer(0.0f) {
  state = EnemyState::PATROL;
  generate_patrol_points(10);
}

Vector3 Enemy::get_random_position(float position_radius) {
  float position_angle =
      GetRandomValue(0, 360) * DEG2RAD; // Random starting angle

  return {position_radius * cosf(position_angle), 3.0f,
          position_radius * sinf(position_angle)};
}
void Enemy::update(float dt, const Vector3 &current_player_position) {
  patrol_point_update_timer += dt;
  if (patrol_point_update_timer >= PATROL_POINT_UPDATE_INTERVAL) {
    patrol_point_update_timer = 0.0f;
    // Replace a random patrol point with player position
    if (!patrol_points.empty()) {
      size_t random_index = GetRandomValue(0, patrol_points.size() - 1);
      Vector3 position_to_target = current_player_position;
      position_to_target.y =
          position.y; // target point is still on same level as enemy
      patrol_points[random_index] = position_to_target;
    }
  }

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
    target_position = last_known_player_position;
    break;
  case EnemyState::RETURN_TO_PATROL:
    // Project target position further in the direction player was last seen
    Vector3 projected_position = last_known_player_position;
    Vector3 direction = Vector3Subtract(last_known_player_position, position);
    direction = Vector3Normalize(direction);
    projected_position =
        Vector3Add(last_known_player_position, Vector3Scale(direction, 10.0f));
    projected_position.y = position.y; // Keep on same height plane

    if (Vector3Distance(position, last_known_player_position) < 2.0f) {
      // Once we reach the last known position, return to normal patrol
      state = EnemyState::PATROL;
      movement_speed = PATROL_SPEED;
    } else {
      target_position = projected_position;
    }
    break;
  }

  // Move towards target position
  Vector3 direction = Vector3Subtract(target_position, position);
  direction = Vector3Normalize(direction);

  // Scale by speed and delta time
  direction.x *= movement_speed * dt;
  direction.z *= movement_speed * dt;

  // Update position and facing direction
  position = Vector3Add(position, direction);
  if (Vector3Length(direction) > 0.0f) {
    facing_direction = Vector3Normalize(direction);
  }

  // Check if player is in vision cone
  // Project player position to enemy's height plane for detection
  Vector3 player_at_enemy_height = current_player_position;
  player_at_enemy_height.y = position.y; // Project onto enemy's height plane

  Vector3 to_player = Vector3Subtract(player_at_enemy_height, position);
  float distance_to_player = Vector3Length(to_player);

  // nested conditions like this are hard to read and generally bad programming
  if (distance_to_player <= vision_range) {
    Vector3 to_player_normalized = Vector3Normalize(to_player);
    float angle =
        acosf(Vector3DotProduct(facing_direction, to_player_normalized));

    float effective_vision_angle = vision_angle;
    if (can_see_player) {
      effective_vision_angle *= 1.1f;
    }

    // if player in vision cone then enemy should be able to see player
    bool in_vision_cone = angle <= effective_vision_angle / 2.0f;
    if (in_vision_cone && collision_checker != nullptr) {
      Ray ray = {position, to_player_normalized};
      // checks if the current enemy vision ray hits an obstacle
      can_see_player =
          !collision_checker->check_collision_ray(ray, distance_to_player)
               .collision;
    } else {
      can_see_player = false;
    }
  } else {
    can_see_player = false;
  }

  // update goap
  update_state(dt, current_player_position);
}

void Enemy::update_state(float dt, const Vector3 &current_player_position) {
  switch (state) {
  case EnemyState::PATROL:
    if (can_see_player) {
      state = EnemyState::CHASE;
      movement_speed = CHASE_SPEED;
      last_known_player_position = current_player_position;
      last_known_player_position.y = position.y;
    }
    break;
  case EnemyState::CHASE:
    if (can_see_player) {
      chase_cooldown_timer = 0.0f; // Reset timer when we can see player
      last_known_player_position = current_player_position;
      last_known_player_position.y = position.y;
    } else {
      chase_cooldown_timer += dt;
      if (chase_cooldown_timer >= 1.0f) {     // 1 second cooldown
        state = EnemyState::RETURN_TO_PATROL; // Change to return state instead
                                              // of patrol
        movement_speed = PATROL_SPEED;
        chase_cooldown_timer = 0.0f;
      }
    }
    break;
  case EnemyState::RETURN_TO_PATROL:
    if (can_see_player) {
      state = EnemyState::CHASE;
      movement_speed = CHASE_SPEED;
      last_known_player_position = current_player_position;
      last_known_player_position.y = position.y;
    }
    break; // Remove the else clause - let the update() method handle the
           // transition to PATROL
  }
}

void Enemy::generate_patrol_points(int n) {
  patrol_points.clear();
  float patrol_radius = 40.0f;

  for (int i = 0; i < n; i++) {
    patrol_points.push_back(get_random_position(patrol_radius));
  }
}

void Enemy::draw() {
  // Draw the enemy sphere
  DrawSphere(position, radius, color);

  // Draw current target point slightly larger
  if (debug) {
    // Draw patrol points for debugging
    for (const auto &point : patrol_points) {
      DrawSphere(point, 0.3f, YELLOW);
    }
    Vector3 target = patrol_points[current_patrol_point_index];
    DrawSphere(target, 0.5f, GREEN);

    // Draw line from enemy to current target
    DrawLine3D(position, target, RED);
  }

  // Draw vision cone
  Color vision_color;
  switch (state) {
  case EnemyState::CHASE:
    vision_color = RED;
    break;
  case EnemyState::RETURN_TO_PATROL:
    vision_color = ORANGE;
    break;
  default: // PATROL state
    vision_color = YELLOW;
    break;
  }

  // Calculate cone points
  Vector3 right_dir = Vector3RotateByAxisAngle(
      facing_direction, (Vector3){0, 1, 0}, vision_angle / 2);
  Vector3 left_dir = Vector3RotateByAxisAngle(
      facing_direction, (Vector3){0, 1, 0}, -vision_angle / 2);

  Vector3 cone_right =
      Vector3Add(position, Vector3Scale(right_dir, vision_range));
  Vector3 cone_left =
      Vector3Add(position, Vector3Scale(left_dir, vision_range));

  // Draw filled vision cone using triangles
  DrawTriangle3D(position, cone_right, cone_left, Fade(vision_color, 0.5f));

  // Draw vision cone outline
  DrawLine3D(position, cone_right, vision_color);
  DrawLine3D(position, cone_left, vision_color);
  DrawLine3D(cone_right, cone_left, vision_color);
}
