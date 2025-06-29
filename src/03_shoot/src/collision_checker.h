#pragma once
#include "raylib.h"

struct CollisionInfo {
  bool collision;
  Vector3 normal;
};

class CollisionChecker {
public:
  virtual ~CollisionChecker() = default;
  // luckily, raylib has nice abstractions for handling rays
  // virtual const = 0 means this is a truly abstract member
  virtual CollisionInfo check_collision_ray(Ray ray,
                                            float max_distance) const = 0;
};
