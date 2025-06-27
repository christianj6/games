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

  EnemyState current_state;
  bool can_see_player;
  Vector3 last_known_player_position;

  float movement_speed_patrol;
  float movement_speed_chase;
  Vector3 current_patrol_target;
};
