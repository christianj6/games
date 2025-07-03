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
      movement_speed_patrol(0.05f), movement_speed_chase(0.2f),
      current_path_index() {
  position = get_random_world_position(2);
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

  // Update rotation to match movement direction
  horizontal_rotation = atan2f(direction.z, direction.x);
  vertical_rotation = asinf(direction.y);

  // Update forward vector based on current rotation
  forward_vector =
      Vector3{cosf(horizontal_rotation), 0.0f, sinf(horizontal_rotation)};

  // Check if we've reached current node
  const float NODE_REACH_THRESHOLD = 0.5f;
  if (distance < NODE_REACH_THRESHOLD) {
    current_path_index++;
  }

  // Always return FAILURE unless we've completed the entire path
  return BT::NodeStatus::FAILURE;
}

BT::NodeStatus
Enemy::shoot_projectile_at_player(const Vector3 &player_position) {
  if (shoot_timer > 0) {
    return BT::NodeStatus::FAILURE;
  }

  // Calculate direction to player
  Vector3 direction = Vector3Subtract(player_position, position);
  direction = Vector3Normalize(direction);

  // Add new projectile
  projectiles.push_back({
      position,  // start at enemy position
      direction, // direction to player
      true       // active
  });

  // Reset shoot timer
  shoot_timer = SHOOT_COOLDOWN;

  return BT::NodeStatus::FAILURE;
}

void Enemy::configure_tree_factory(
    const std::vector<Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>>
        &world_data,
    const Vector3 &current_player_position) {
  factory = std::make_unique<BT::BehaviorTreeFactory>();
  factory->registerSimpleAction("FindNewPath", [&](BT::TreeNode &) {
    return generate_new_path(world_data);
  });
  factory->registerSimpleAction("FollowPath", [&](BT::TreeNode &) {
    return move_towards_next_path_node();
  });
  factory->registerSimpleAction("GetPathtoPlayer", [&](BT::TreeNode &) {
    return generate_path_to_player(world_data, current_player_position);
  });
  factory->registerSimpleAction("ShootPlayer", [&](BT::TreeNode &) {
    return shoot_projectile_at_player(current_player_position);
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

EnemySignals Enemy::update(
    float dt, const Vector3 &current_player_position,
    const std::vector<Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>>
        &world_data,
    bool any_enemy_can_see_player) {
  if (current_state == EnemyState::DEAD) {
    // if enemy is dead do nothing
    return {false, false};
  }
  // set some useful variables
  bool is_killable = false;
  can_see_player = is_in_vision_cone(current_player_position);
  float distance_to_player =
      Vector3Length(Vector3Subtract(current_player_position, position));
  const float min_distance_to_player = 15.0f;

  // Update shoot timer
  if (shoot_timer > 0) {
    shoot_timer -= GetFrameTime();
  }

  bool projectile_hit = false;
  // Update existing projectiles
  for (auto &proj : projectiles) {
    if (proj.active) {
      proj.position = Vector3Add(
          proj.position, Vector3Scale(proj.direction, PROJECTILE_SPEED));

      // Check collision with player
      float dist_to_player = Vector3Length(
          Vector3Subtract(proj.position, current_player_position));
      if (dist_to_player < 1.0f) { // Using 1.0 as collision radius
        proj.active = false;
        projectile_hit = true;
        continue;
      }

      // Deactivate if too far
      if (Vector3Length(Vector3Subtract(proj.position, position)) > 50.0f) {
        proj.active = false;
      }
    }
  }

  // Remove inactive projectiles
  projectiles.erase(
      std::remove_if(projectiles.begin(), projectiles.end(),
                     [](const Projectile &p) { return !p.active; }),
      projectiles.end());

  // main behavior logic using behavior_trees
  if (tree) {
    BT::NodeStatus result = tree->tickOnce();
    switch (result) {
    case (BT::NodeStatus::SUCCESS):
      // reset tree once we have reached the end
      tree.reset();
      break;
    case (BT::NodeStatus::FAILURE):
      // signifies an in progress tree
      break;
    // we never engage these statuses
    // but keep them here to keep lsp happy
    case (BT::NodeStatus::RUNNING):
      break;
    case (BT::NodeStatus::IDLE):
      break;
    case (BT::NodeStatus::SKIPPED):
      break;
    }
  } else {
    // mapping from states to behavior trees
    switch (current_state) {
    case (EnemyState::CHASING):
      configure_tree_factory(world_data, current_player_position);
      tree = std::make_unique<BT::Tree>(
          factory->createTreeFromFile("behavior_trees/enemy_chasing.xml"));
      break;
    case (EnemyState::SEARCHING):
      configure_tree_factory(world_data, current_player_position);
      tree = std::make_unique<BT::Tree>(
          factory->createTreeFromFile("behavior_trees/enemy_searching.xml"));
      break;
    case (EnemyState::DEAD):
      break;
    case (EnemyState::PATROLLING):
      // configure the tree here bc of some caching issues
      configure_tree_factory(world_data, current_player_position);
      tree = std::make_unique<BT::Tree>(
          factory->createTreeFromFile("behavior_trees/enemy_patrolling.xml"));
      break;
    }
  }
  // this logic is really messy and hard to comprehend
  // we have some nice tools here but need a better concept
  // for the next project

  // state transitions
  if (current_state != EnemyState::CHASING && any_enemy_can_see_player &&
      distance_to_player <= min_distance_to_player) {
    tree.reset();
    current_state = EnemyState::CHASING;
  }
  if (current_state == EnemyState::PATROLLING && can_see_player) {
    tree.reset();
    current_state = EnemyState::CHASING;
  }
  if (current_state == EnemyState::CHASING && !can_see_player &&
      !any_enemy_can_see_player) {
    tree.reset();
    current_state = EnemyState::SEARCHING;
    search_timer = 0.0f; // Reset timer when entering search state
  }
  if (current_state == EnemyState::SEARCHING) {
    search_timer += dt; // Update timer while searching
    if (search_timer > SEARCH_TIMER_MAX) {
      tree.reset();
      current_state = EnemyState::PATROLLING;
    }
  }

  // in the future we can also represent killability via
  // a state like vulnerable (tricky because that would be multiple states)

  // color coding
  if (distance_to_player <= 3.5f) {
    is_killable = true;
    color = RED;
    color.a = 100;
  } else {
    if (current_state == EnemyState::PATROLLING) {
      color = DARKGRAY;
    }
    if (current_state == EnemyState::CHASING) {
      color = RED;
    }
    if (current_state == EnemyState::SEARCHING) {
      color = YELLOW;
    }
  }

  return {can_see_player, is_killable, projectile_hit};
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

      // Calculate base angles
      float h_angle_world = h_angle + horizontal_rotation;

      // Calculate point on the cone surface
      Vector3 direction = {
          cosf(v_angle) * cosf(h_angle_world), // x
          sinf(v_angle),                       // y
          cosf(v_angle) * sinf(h_angle_world)  // z
      };

      Vector3 end_point =
          Vector3Add(position, Vector3Scale(direction, vision_range));

      // Draw lines to create a wireframe effect
      Color cone_color = can_see_player ? RED : YELLOW;
      if (h < horizontal_segments) {
        // Calculate next horizontal point
        float next_h_angle_world =
            (-h_half_angle + (horizontal_fov * (h + 1) / horizontal_segments)) +
            horizontal_rotation;
        Vector3 next_direction = {cosf(v_angle) * cosf(next_h_angle_world),
                                  sinf(v_angle),
                                  cosf(v_angle) * sinf(next_h_angle_world)};
        Vector3 next_point =
            Vector3Add(position, Vector3Scale(next_direction, vision_range));
        DrawLine3D(end_point, next_point, ColorAlpha(cone_color, 0.3f));
      }

      if (v < vertical_segments) {
        // Calculate next vertical point
        float next_v_angle =
            -v_half_angle + (vertical_fov * (v + 1) / vertical_segments);
        Vector3 next_direction = {cosf(next_v_angle) * cosf(h_angle_world),
                                  sinf(next_v_angle),
                                  cosf(next_v_angle) * sinf(h_angle_world)};
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
  if (current_state != EnemyState::DEAD) {
    DrawSphere(position, radius, color);
    // Draw active projectiles
    for (const auto &proj : projectiles) {
      if (proj.active) {
        DrawSphere(proj.position, 0.3f, RED);
      }
    }
    // draw_vision_cone();
    // DrawSphere(current_patrol_target, 1.0f, YELLOW);
    // draw_current_path();
    if (current_state != EnemyState::CHASING) {
      Vector3 beam_end = Vector3Add(
          position, Vector3Scale(Vector3{cosf(horizontal_rotation), 0.0f,
                                         sinf(horizontal_rotation)},
                                 3.0f));
      DrawLine3D(position, beam_end, YELLOW);
    }
  }
}

void Enemy::draw_current_path() {
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

BT::NodeStatus Enemy::generate_new_path(
    const std::vector<Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>>
        &world_data) {
  current_patrol_target = get_random_unobstructed_world_position(1, world_data);
  current_patrol_target.y += 1;

  micropather::MPVector<void *> raw_path;
  float totalCost = 0;

  Node startNode(position.x, position.z);
  Node endNode(current_patrol_target.x, current_patrol_target.z);

  if (!startNode.IsValid() || !endNode.IsValid()) {
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
    return BT::NodeStatus::SUCCESS;
  }
  return BT::NodeStatus::FAILURE;
}

BT::NodeStatus Enemy::generate_path_to_player(
    const std::vector<Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic>>
        &world_data,
    const Vector3 &current_player_position) {
  micropather::MPVector<void *> raw_path;
  float totalCost = 0;

  Node startNode(position.x, position.z);
  Vector3 position_close_to_player = find_unobstructed_position_near(
      Vector2{current_player_position.x, current_player_position.z},
      world_data);
  Node endNode(position_close_to_player.x, position_close_to_player.z);

  if (!startNode.IsValid() || !endNode.IsValid()) {
    // return BT::NodeStatus::FAILURE;
    return BT::NodeStatus::SUCCESS;
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
    return BT::NodeStatus::SUCCESS;
  }
  return BT::NodeStatus::FAILURE;
}
