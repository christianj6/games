#pragma once

#include "ai/pathfinder.h"
#include "behaviortree_cpp/bt_factory.h"
#include "raylib.h"
#include <Eigen/Dense>
#include <behaviortree_cpp/basic_types.h>
#include <memory>

enum class EnemyState { PATROLLING, CHASING, SEARCHING, DEAD };

class Enemy {
public:
  Enemy(micropather::MicroPather *);
  ~Enemy(){};
  Enemy(Enemy &&) = default; // Add move constructor for BT
  bool update(
      float, const Vector3 &,
      const std::vector<Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>> &);
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

  // behavior tree stuff
  std::unique_ptr<BT::Tree> tree;

  // pathfinding
  // micropather::MPVector<void *> current_path;
  std::vector<Vector3> current_path;
  size_t current_path_index = 0;
  micropather::MicroPather *pather;
  std::unique_ptr<BT::BehaviorTreeFactory> factory;
  void configure_tree_factory(
      const std::vector<Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>> &);
  void draw_current_path();
  // utils for pathfinding
  BT::NodeStatus move_towards_next_path_node();
  BT::NodeStatus generate_new_path(
      const std::vector<Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>>
          &world_data);
};
