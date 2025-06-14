#include "enemy.h"
#include "raylib.h"
#include "raymath.h"
#include <cmath>

Enemy::Enemy(Vector2 starting_position, int num_patrol_points)
    : GameObject(starting_position) {
  current_state = EnemyState::PATROL;
  vision_angle = PI / 3.0f; // 60 degrees
  vision_range = 600.0f;
  rotation = 0.0f;
  move_speed = 450.0f;
  rotation_speed = PI; // radians per second
  current_patrol_index = 0;
  can_see_player = false;
  search_timeout = 3.0f; // 3 seconds to search last known position
  search_timer = 0.0f;
  patrol_update_timer = 0.0f;
  patrol_update_interval = 4.0f; // Update a patrol point every 4 seconds

  generate_patrol_points(num_patrol_points);
}

bool Enemy::is_player_in_vision_cone(
    const Vector2 &player_pos,
    // because cpp syntax is a little cursed, you can put the & or * either to
    // the right of the type or to the left of the variable name
    const std::vector<GameObject *> &obstacles) const {
  // get direction from enemy to player
  Vector2 to_player = Vector2Subtract(player_pos, position);
  float distance = Vector2Length(to_player);

  if (distance > vision_range)
    return false;

  float angle_to_player = atan2f(to_player.y, to_player.x);
  float angle_diff = fabs(angle_to_player - rotation);
  // ensure angle is always the smallest (handle wrap around case)
  while (angle_diff > PI)
    angle_diff = 2 * PI - angle_diff;

  if (angle_diff > vision_angle / 2)
    // if the smallest angle to the player is greater than half of the vision
    // cone size, means player is outside of the cone
    return false;

  return has_line_of_sight(player_pos, obstacles);
}

bool Enemy::has_line_of_sight(
    const Vector2 &target_pos,
    const std::vector<GameObject *> &obstacles) const {
  Vector2 to_target = Vector2Subtract(target_pos, position);
  Vector2 ray_dir = Vector2Normalize(to_target);
  float distance_to_target = Vector2Length(to_target);

  for (const auto *obstacle : obstacles) {
    Vector2 to_obstacle = Vector2Subtract(obstacle->get_position(), position);
    float dist_to_obstacle = Vector2Length(to_obstacle);

    if (dist_to_obstacle < distance_to_target) {
      float dot = Vector2DotProduct(to_obstacle, ray_dir);
      Vector2 closest_point = Vector2Add(position, Vector2Scale(ray_dir, dot));

      if (Vector2Distance(closest_point, obstacle->get_position()) < 125.0f) {
        return false;
      }
    }
  }
  return true;
}

void Enemy::update_goap(float dt, const Vector2 &player_pos,
                        const std::vector<GameObject *> &obstacles) {
  can_see_player = is_player_in_vision_cone(player_pos, obstacles);

  // Update patrol points occasionally
  patrol_update_timer += dt;
  if (patrol_update_timer >= patrol_update_interval) {
    patrol_update_timer = 0.0f;
    // Replace a random patrol point with the player's position
    if (!patrol_points.empty()) {
      size_t random_index = GetRandomValue(0, patrol_points.size() - 1);
      patrol_points[random_index] = player_pos;
    }
  }

  switch (current_state) {
  case EnemyState::PATROL:
    if (can_see_player) {
      current_state = EnemyState::CHASE;
      last_known_player_pos = player_pos;
    }
    break;

  case EnemyState::CHASE:
    if (!can_see_player) {
      current_state = EnemyState::RETURN_TO_PATROL;
    } else {
      last_known_player_pos = player_pos;
    }
    break;

  case EnemyState::RETURN_TO_PATROL:
    if (can_see_player) {
      current_state = EnemyState::CHASE;
      last_known_player_pos = player_pos;
    } else if (Vector2Distance(position, patrol_points[current_patrol_index]) <
               10.0f) {
      current_state = EnemyState::PATROL;
    }
    break;
  }
}

void Enemy::update(float dt, bool allow_movement) {
  if (!allow_movement)
    return;

  // define a vector target pos representing where the enemy is currently going
  Vector2 target_pos;
  switch (current_state) {
  case EnemyState::PATROL:
    target_pos = patrol_points[current_patrol_index];
    if (Vector2Distance(position, target_pos) < 10.0f) {
      current_patrol_index = (current_patrol_index + 1) % patrol_points.size();
    }
    search_timer = 0.0f; // Reset timer when patrolling
    break;

  case EnemyState::CHASE:
    target_pos = last_known_player_pos;
    search_timer = 0.0f; // Reset timer when chasing
    break;

  case EnemyState::RETURN_TO_PATROL:
    if (Vector2Distance(position, last_known_player_pos) > 10.0f &&
        search_timer < search_timeout) {
      // Still heading to last known position and haven't timed out
      target_pos = last_known_player_pos;
      search_timer += dt;
    } else {
      // Either reached the position or timed out, return to patrol
      target_pos = patrol_points[current_patrol_index];
    }
    break;
  }

  // Calculate direction to target
  Vector2 direction = Vector2Subtract(target_pos, position);
  // atan2f lets you convert a vector to an angle (radians)
  float target_rotation = atan2f(direction.y, direction.x);

  // Smoothly rotate towards target
  float angle_diff = target_rotation - rotation;
  while (angle_diff > PI)
    angle_diff -= 2 * PI;
  while (angle_diff < -PI)
    angle_diff += 2 * PI;

  rotation +=
      (angle_diff > 0 ? 1 : -1) * fminf(fabsf(angle_diff), rotation_speed * dt);

  // Move in facing direction
  position.x += cosf(rotation) * move_speed * dt;
  position.y += sinf(rotation) * move_speed * dt;
}

void Enemy::draw() {
  // Draw enemy body
  DrawCircle(position.x, position.y, 30, RED);

  // Draw vision cone
  Color vision_color = can_see_player ? YELLOW : GRAY; // ternary operator
  vision_color.a = 100;

  float start_angle = rotation - vision_angle / 2;
  float end_angle = rotation + vision_angle / 2;
  DrawCircleSector(position, vision_range, start_angle * RAD2DEG,
                   end_angle * RAD2DEG, 32, vision_color);
}

void Enemy::draw_indicator(Vector2 player_position) {
  // Direction in world space
  Vector2 dir = Vector2Subtract(position, player_position);

  // Avoid zero-length vectors
  if (Vector2Length(dir) < 0.01f)
    return;

  // Get screen center (representing player position)
  Vector2 screen_center = {GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};

  // Calculate normalized direction and scale it to desired distance from center
  Vector2 screen_dir = Vector2Normalize(dir);
  float indicator_distance = 40.0f;

  // Calculate indicator position
  Vector2 indicator_pos =
      Vector2Add(screen_center, Vector2Scale(screen_dir, indicator_distance));

  // Draw direction indicator dot
  DrawCircle(indicator_pos.x, indicator_pos.y, 5, RED);
}

bool Enemy::is_touching_player(const Vector2 &player_pos) const {
  return CheckCollisionCircles(position, 30, player_pos, 40);
}

void Enemy::set_patrol_points(const std::vector<Vector2> &points) {
  patrol_points = points;
  current_patrol_index = 0;
}

void Enemy::generate_patrol_points(int num_points) {
  patrol_points.clear();

  // Generate points in a semi-random pattern around the starting position
  float radius = 1000.0f; // Base radius for patrol area
  float angle_step = (2 * PI) / num_points;

  for (int i = 0; i < num_points; i++) {
    float angle = angle_step * i;
    // Add some randomness to both radius and angle
    float random_radius = radius * (0.8f + (GetRandomValue(0, 40) / 100.0f));
    float random_angle = angle + (GetRandomValue(-10, 10) / 100.0f);

    Vector2 point = {position.x + random_radius * cosf(random_angle),
                     position.y + random_radius * sinf(random_angle)};
    patrol_points.push_back(point);
  }
}
