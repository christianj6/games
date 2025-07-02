#include "enemy.h"
#include "fmt/core.h"
#include "raylib.h"
#include "raymath.h"
#include "utils/random.h"
#include <behaviortree_cpp/basic_types.h>
#include <behaviortree_cpp/bt_factory.h>
#include <fmt/base.h>
#include <memory>

Enemy::Enemy(micropather::MicroPather *pather)
    : radius(1.0f), color(DARKGRAY), current_state(EnemyState::PATROLLING),
      horizontal_rotation(0.0f), vertical_rotation(0.0f),
      horizontal_fov(PI / 2.0f), vertical_fov(PI / 3.0f), vision_range(20.0f),
      forward_vector({1.0f, 0.0f, 0.0f}), pather(pather),
      movement_speed_patrol(0.2f), movement_speed_chase(0.5f),
      current_path_index() {
  position = get_random_world_position(2);
  // configure_tree_factory();
}

BT::NodeStatus Enemy::move_towards_next_path_node() {
  // Check if we have a valid path
  if (current_path.size() == 0) {
    return BT::NodeStatus::FAILURE;
  }

  // Check if we've completed the entire path
  if (current_path_index >= current_path.size()) {
    current_path_index = 0;
    return BT::NodeStatus::SUCCESS;
  }

  // Move towards current target node
  Vector3 target_position = current_path[current_path_index];
  Vector3 direction = Vector3Subtract(target_position, position);
  float distance = Vector3Length(direction);
  direction = Vector3Normalize(direction);
  position =
      Vector3Add(position, Vector3Scale(direction, movement_speed_patrol));
  forward_vector = Vector3Normalize(Vector3{direction.x, 0.0f, direction.z});

  // Check if we've reached current node
  const float NODE_REACH_THRESHOLD = 0.5f;
  if (distance < NODE_REACH_THRESHOLD) {
    current_path_index++;
    // fmt::println("{}", current_path_index);
    // fmt::println("{}", current_path.size());
  }

  // Always return FAILURE unless we've completed the entire path
  return BT::NodeStatus::FAILURE;
}

void Enemy::configure_tree_factory(
    const std::vector<Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>>
        &world_data) {
  factory = std::make_unique<BT::BehaviorTreeFactory>();
  factory->registerSimpleAction("FindNewPath", [&](BT::TreeNode &) {
    return generate_new_path(world_data);
  });
  factory->registerSimpleAction("FollowPath", [&](BT::TreeNode &) {
    return move_towards_next_path_node();
  });
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

bool Enemy::update(
    float dt, const Vector3 &current_player_position,
    const std::vector<Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>>
        &world_data) {
  if (tree) {
    // fmt::print("trying to tick\n");
    BT::NodeStatus result = tree->tickOnce();
    // fmt::print("result!");
    // decide what to do based on result of tree tick
    switch (result) {
    case (BT::NodeStatus::SUCCESS):
      // reached the end
      tree.reset();
      break;
    case (BT::NodeStatus::FAILURE):
      // in progress
      break;
    // we never engage these states
    // but keep them here to keep lsp happy
    case (BT::NodeStatus::RUNNING):
      break;
    case (BT::NodeStatus::IDLE):
      break;
    case (BT::NodeStatus::SKIPPED):
      break;
    }
  } else {
    // transition between states
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
      // fmt::println("building a new tree");
      // put the random point selection here for convenience
      // fmt::print("{}, {}\n", current_patrol_target.x,
      // current_patrol_target.z);
      configure_tree_factory(world_data);
      tree = std::make_unique<BT::Tree>(
          factory->createTreeFromFile("behavior_trees/enemy_patrolling.xml"));
      // generate_new_path();
      break;
    }
  }
  // killability
  bool is_killable = false;
  float distance_to_player =
      Vector3Length(Vector3Subtract(current_player_position, position));
  can_see_player = is_in_vision_cone(current_player_position);
  if (distance_to_player <= 3.5f) {
    color = RED;
    is_killable = true;
  } else {
    color = DARKGRAY;
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
    // draw_vision_cone();
    DrawSphere(current_patrol_target, 1.0f, YELLOW);
    // draw_current_path();
  }
}

void Enemy::draw_current_path() {
  // // Need at least 2 points to draw a path
  // if (current_path.size() < 2)
  //   return;
  //
  // for (unsigned i = 0; i < current_path.size() - 1; ++i) {
  //   // Skip invalid state pointers
  //   if (!current_path[i] || !current_path[i + 1])
  //     continue;
  //
  //   // Convert path points to nodes
  //   Node current = Node::FromState(current_path[i]);
  //   Node next = Node::FromState(current_path[i + 1]);
  //
  //   // Draw path segment and waypoint marker
  //   DrawLine3D({current.x * 1.0f, 3.0f, current.y * 1.0f},
  //              {next.x * 1.0f, 3.0f, next.y * 1.0f}, YELLOW);
  //   DrawSphere({current.x * 1.0f, 3.0f, current.y * 1.0f}, 0.5f, BLUE);
  // }
  if (current_path.size() < 2) {
    return;
  }
  for (unsigned i = 0; i < current_path.size() - 1; ++i) {
    Vector3 current = current_path[i];
    Vector3 next = current_path[i + 1];

    DrawLine3D({current.x, 3.0f, current.z}, {next.x, 3.0f, next.z}, YELLOW);
    DrawSphere({current.x, 3.0f, current.z}, 0.5f, BLUE);
  }
}

// BT::NodeStatus Enemy::generate_new_path() {
//   fmt::print("find new path");
//   // reset pathfinding variables
//   // pather->Reset();
//   current_path = micropather::MPVector<void *>();
//   float totalCost = 0;
//
//   Node startNode(position.x, position.z);
//   Node endNode(current_patrol_target.x, current_patrol_target.z);
//   if (!startNode.IsValid() || !endNode.IsValid()) {
//     return BT::NodeStatus::FAILURE;
//   }
//   void *startState = startNode.ToState();
//   void *endState = endNode.ToState();
//   int result = pather->Solve(startState, endState, &current_path,
//   &totalCost); if (result == micropather::MicroPather::SOLVED) {
//     return BT::NodeStatus::SUCCESS;
//   }
//   return BT::NodeStatus::FAILURE;
// }

BT::NodeStatus Enemy::generate_new_path(
    const std::vector<Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>>
        &world_data) {
  // fmt::print("find new path\n");
  current_patrol_target = get_random_unobstructed_world_position(1, world_data);
  current_patrol_target.y += 1;

  micropather::MPVector<void *> raw_path;
  float totalCost = 0;

  Node startNode(position.x, position.z);
  Node endNode(current_patrol_target.x, current_patrol_target.z);

  if (!startNode.IsValid() || !endNode.IsValid()) {
    // fmt::print("{}, {}, {}, {}\n", position.x, position.z,
    //            current_patrol_target.z, current_patrol_target.z);
    fmt::println("failed path valid");
    return BT::NodeStatus::FAILURE;
  }

  void *startState = startNode.ToState();
  void *endState = endNode.ToState();

  int result = pather->Solve(startState, endState, &raw_path, &totalCost);

  if (result == micropather::MicroPather::SOLVED) {
    current_path.clear();
    for (size_t i = 0; i < raw_path.size(); ++i) {
      Node node = Node::FromState(raw_path[i]);
      current_path.push_back(Vector3{static_cast<float>(node.x), position.y,
                                     static_cast<float>(node.y)});
    }
    current_path_index = 0;
    // return BT::NodeStatus::FAILURE;
    fmt::println("found a path");
    return BT::NodeStatus::SUCCESS;
  }
  fmt::println("failed path default");
  // abort
  return BT::NodeStatus::FAILURE;
}
