#pragma once
#include "enemy.h"
#include "obstacle.h"
#include "raylib.h"
#include "wall.h"
#include "weapon.h"
#include <memory>
#include <vector>

#include "collision_checker.h"

// world inherits from the collision checker so it can implement the
// check_collision_ray
class World : public CollisionChecker {
public:
  World();
  void update(float, const Vector3 &);
  void draw();
  CollisionInfo check_collision(const Vector3 &position) const;
  CollisionInfo check_collision_ray(Ray ray, float max_distance)
      const; // we could add override here to be more explicit
  // means that the member should not be modified directly (we can manipulate it
  // using another function below) return type is a container of unique pointers
  // to the Enemy type
  const std::vector<std::unique_ptr<Enemy>> &get_enemies() const {
    return enemies;
  }

  // size_t is the preferred type for indexes bc it scales to the system
  // architecture
  void remove_enemy(size_t index) {
    if (index < enemies.size()) {
      enemies.erase(enemies.begin() + index);
    }
  }

  // .get provides the raw pointer but does not transfer ownership
  const Weapon *get_weapon() const { return weapon.get(); }
  // .reset effectively deletes the current object, but has some risk of
  // undefined behavior
  void consume_weapon() { weapon.reset(); }

private:
  std::vector<std::unique_ptr<Obstacle>> obstacles;
  std::vector<std::unique_ptr<Wall>> walls;
  std::vector<std::unique_ptr<Enemy>> enemies;
  std::unique_ptr<Weapon> weapon;
};
