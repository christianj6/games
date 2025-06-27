#pragma once

#include "raylib.h"

enum class EnemyState { PATROLLING, CHASING, SEARCHING, DEAD };

class Enemy {
public:
  Enemy();
  bool update(float, Vector3 &);
  void draw();
  Vector3 get_position() { return position; }
  void disable();

private:
  Vector3 position;
  Color color;
  float radius;

  // Vision cone parameters
  float horizontal_rotation; // Horizontal facing angle in radians
  float vertical_rotation;   // Vertical facing angle in radians
  float horizontal_fov;      // Horizontal field of view in radians
  float vertical_fov;        // Vertical field of view in radians
  float vision_range;        // How far the enemy can see
  Vector3 forward_vector;    // Current facing direction

  EnemyState current_state;
  bool can_see_player;
  Vector3 last_known_player_position;

  float movement_speed_patrol;
  float movement_speed_chase;
  Vector3 current_patrol_target;

  // Helper methods for vision cone
  void update_forward_vector();
  bool is_in_vision_cone(const Vector3 &target) const;
  void draw_vision_cone() const;
};
