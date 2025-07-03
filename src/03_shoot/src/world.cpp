#include "world.h"
#include "raylib.h"
#include "raymath.h"
#include "weapon.h"
#include <memory>
#include <random>

const float playing_field_size = 100.f;

/*
 * used to make sure obstacles do not spawn on top of one another
 */
bool check_obstacle_collision(const Vector3 &pos1, float height1,
                              const Vector3 &pos2, float height2) {
  // Define collision box dimensions
  const float width = 2.0f; // Standard width for obstacles
  float radius1 = width / 2.0f;
  float radius2 = width / 2.0f;

  // Check for cylinder-cylinder intersection
  float dx = pos1.x - pos2.x;
  float dz = pos1.z - pos2.z;
  float distance = sqrt(dx * dx + dz * dz);

  return distance < (radius1 + radius2);
}

/*
 * used to randomly place obstacles (and the weapon pickpu)
 */
Vector3 get_random_position(float height) {
  // margin prevents obstacles from spawning too close to the walls
  const float margin = 5.0f;
  const float min_pos = (-playing_field_size / 2.0f) + margin;
  const float max_pos = (playing_field_size / 2.0f) - margin;

  return (Vector3){(float)GetRandomValue((int)min_pos, (int)max_pos),
                   height / 2.0f,
                   (float)GetRandomValue((int)min_pos, (int)max_pos)};
}

Color get_random_color() {
  // obstacle colors (you could just as well use an array in this scenario)
  std::vector<Color> colors = {GRAY, RAYWHITE, DARKPURPLE, BLACK};
  // a generator is an object that produces a sequence of numbers over time when
  // called (similar to python concept) you can implement your own generator by
  // just implementing an operator() on your class by prefixing the statement
  // with static, we say that we want to use the same object across function
  // calls (more efficient and more random)
  static std::mt19937 gen(std::random_device{}());
  // make a distribution which will convert our random numbers into values in
  // the range we want
  std::uniform_int_distribution<> dist(0, colors.size() - 1);
  // use the distribution to call gen and get a random value for indexing the
  // vector of colors
  return colors[dist(gen)];
}

World::World() : obstacles(), walls(), enemies(), weapon(nullptr) {
  // First create weapon
  Vector3 weapon_pos = get_random_position(3.0f);
  weapon = std::make_unique<Weapon>(weapon_pos);
  // TODO: make sure weapon does not collide with candidate obstacle positions

  // obstacles
  const int n = 100;
  const int max_attempts = 100; // Maximum attempts to place each obstacle
  for (int i = 0; i < n; i++) {
    float height = GetRandomValue(1, 12);
    Vector3 position;
    bool valid_position = false;

    // Try to find a non-colliding position
    for (int attempt = 0; attempt < max_attempts; attempt++) {
      position = get_random_position(height);
      valid_position = true;

      // Check against all existing obstacles
      for (const auto &obstacle : obstacles) {
        if (check_obstacle_collision(position, height, obstacle->get_position(),
                                     obstacle->get_height())) {
          valid_position = false;
          break;
        }
      }

      if (valid_position)
        // if this candidate obstacle position does not collide w any others,
        // continue and place it
        break;
    }

    Color color = get_random_color();
    obstacles.push_back(std::make_unique<Obstacle>(height, position, color));
  }
  // Add some enemies
  const int num_enemies = 6;
  for (int i = 0; i < num_enemies; i++) {
    auto enemy = std::make_unique<Enemy>();
    // set this object on the enemy as a kind of weird way to expose the
    // collision checking functionality another example of mixed responsibility
    // which should be cleaned up in future projects
    enemy->set_collision_checker(this);
    // transfer ownership to the container
    enemies.push_back(std::move(enemy));
  }

  // walls
  // values like this should be extracted; it is fine to inject when creating
  // the walls but probs don't hide them in here
  const float wall_height = 50.0f;
  const Color wall_color = DARKGRAY;
  const float half_size = playing_field_size / 2.0f;

  // North wall (along Z axis)
  walls.push_back(std::make_unique<Wall>(
      wall_height, Vector3{0.0f, wall_height / 2, -half_size}, wall_color,
      playing_field_size,
      true // rotated 90 degrees
      ));

  // South wall (along Z axis)
  walls.push_back(std::make_unique<Wall>(
      wall_height, Vector3{0.0f, wall_height / 2, half_size}, wall_color,
      playing_field_size,
      true // rotated 90 degrees
      ));

  // East wall (along X axis)
  walls.push_back(std::make_unique<Wall>(
      wall_height, Vector3{half_size, wall_height / 2, 0.0f}, wall_color,
      playing_field_size));

  // West wall (along X axis)
  walls.push_back(std::make_unique<Wall>(
      wall_height, Vector3{-half_size, wall_height / 2, 0.0f}, wall_color,
      playing_field_size));
}

void World::draw() {
  DrawPlane((Vector3){0.0f, 0.0f, 0.0f},
            (Vector2){playing_field_size, playing_field_size}, LIGHTGRAY);

  if (weapon && weapon.get()) {
    weapon->draw();
  }
  for (auto &obstacle : obstacles) {
    obstacle->draw();
  }
  for (auto &wall : walls) {
    wall->draw();
  }
  for (auto &enemy : enemies) {
    enemy->draw();
  }
}

CollisionInfo World::check_collision(const Vector3 &position) const {
  // misplaced responsibilities like this are all over the codebase
  // in next project you need much better encapsulation, SRP, etc.
  // in general you also want to take these low-level collision detection
  // concerns out of these game logic layer objects
  const float PLAYER_RADIUS = 0.5f;
  CollisionInfo result = {false, {0, 0, 0}};

  // Check wall collisions
  for (const auto &wall : walls) {
    Vector3 wall_pos = wall->position;
    if (wall->rotate90) {
      // Wall along X axis
      float dist_z = position.z - wall_pos.z;
      float dist_x = fabs(position.x - wall_pos.x);
      if (fabs(dist_z) < (1.0f + PLAYER_RADIUS) &&
          dist_x < (wall->length / 2 + PLAYER_RADIUS)) {
        result.collision = true;
        result.normal = {0, 0, (dist_z > 0) ? 1.0f : -1.0f};
        return result;
      }
    } else {
      // Wall along Z axis
      float dist_x = position.x - wall_pos.x;
      float dist_z = fabs(position.z - wall_pos.z);
      if (fabs(dist_x) < (1.0f + PLAYER_RADIUS) &&
          dist_z < (wall->length / 2 + PLAYER_RADIUS)) {
        result.collision = true;
        result.normal = {(dist_x > 0) ? 1.0f : -1.0f, 0, 0};
        return result;
      }
    }
  }

  // Check obstacle collisions
  for (const auto &obstacle : obstacles) {
    Vector3 obs_pos = obstacle->get_position();
    float dx = position.x - obs_pos.x;
    float dz = position.z - obs_pos.z;
    float distance = sqrt(dx * dx + dz * dz);
    if (distance < (1.0f + PLAYER_RADIUS)) {
      result.collision = true;
      // Calculate normal from obstacle center to player
      result.normal = Vector3Normalize({dx, 0, dz});
      return result;
    }
  }

  return result;
}

/*
  primarily used to check if player is in enemy line of sight
*/
CollisionInfo World::check_collision_ray(Ray ray, float max_distance) const {
  CollisionInfo result = {false, {0, 0, 0}};

  // Check obstacles
  for (const auto &obstacle : obstacles) {
    Vector3 pos = obstacle->get_position();
    float height = obstacle->get_height();

    BoundingBox box = {
        {pos.x - 1.0f, 0.0f, pos.z - 1.0f},  // min point
        {pos.x + 1.0f, height, pos.z + 1.0f} // max point
    };

    RayCollision collision = GetRayCollisionBox(ray, box);
    if (collision.hit && collision.distance < max_distance) {
      result.collision = true;
      result.normal = collision.normal;
      return result;
    }
  }

  // Check walls
  // doesn't really make sense because it is not possible for the character to
  // go beyond walls anyhow
  for (const auto &wall : walls) {
    BoundingBox box;
    if (wall->rotate90) {
      box = {
          {wall->position.x - wall->length / 2, 0.0f, wall->position.z - 1.0f},
          {wall->position.x + wall->length / 2, wall->position.y,
           wall->position.z + 1.0f}};
    } else {
      box = {
          {wall->position.x - 1.0f, 0.0f, wall->position.z - wall->length / 2},
          {wall->position.x + 1.0f, wall->position.y,
           wall->position.z + wall->length / 2}};
    }

    RayCollision collision = GetRayCollisionBox(ray, box);
    if (collision.hit && collision.distance < max_distance) {
      result.collision = true;
      result.normal = collision.normal;
      return result;
    }
  }

  return result;
}

void World::update(float dt, const Vector3 &current_player_position) {
  // Check if any enemies are null before updating
  for (auto it = enemies.begin(); it != enemies.end();) {
    if (*it) {
      (*it)->update(dt, current_player_position);
      ++it;
    } else {
      it = enemies.erase(it);
    }
  }
}
