#pragma once
#include "raylib.h"
#include <vector>

enum class EnemyState { PATROL, CHASE, RETURN_TO_PATROL };

class Enemy {
public:
  Enemy();
  void update(float dt, const Vector3 &);
  void draw();
  Vector3 get_position() const { return position; }

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
  // movement
  std::vector<Vector3> patrol_points;
  void generate_patrol_points(int);
  Vector3 get_random_position(float);
  size_t current_patrol_point_index;
  float movement_speed;
  // debug
  bool debug;
};
