#pragma once
#include "raylib.h"
#include <vector>

#include "collision_checker.h"

enum class EnemyState { PATROL, CHASE, RETURN_TO_PATROL };

class Enemy {
public:
  Enemy();
  void update(float dt, const Vector3 &);
  void draw();
  Vector3 get_position() const { return position; }

public:
  bool is_seeing_player() const { return can_see_player; }
  EnemyState get_state() const { return state; }

private:
  // draw properties
  Vector3 position;
  float radius;
  Color color;
  // vision cone
  float vision_angle; // in radians
  float vision_range;
  Vector3 facing_direction;
  // state management
  EnemyState state;
  void update_state(float, const Vector3 &);
  bool can_see_player;
  Vector3 last_known_player_position;
  float chase_cooldown_timer; // Timer for chase state
  // movement
  std::vector<Vector3> patrol_points;
  void generate_patrol_points(int);
  Vector3 get_random_position(float);
  size_t current_patrol_point_index;
  float movement_speed;
  // debug
  bool debug;
  const CollisionChecker *collision_checker = nullptr;
  float patrol_point_update_timer = 0.0f;
  const float PATROL_POINT_UPDATE_INTERVAL = 5.0f; // 5 seconds

public:
  void set_collision_checker(const CollisionChecker *checker) {
    collision_checker = checker;
  }
};
