#pragma once
#include "raylib.h"

struct CollisionInfo {
  bool collision;
  Vector3 normal;
};

class CollisionChecker {
public:
  virtual ~CollisionChecker() = default;
  virtual CollisionInfo check_collision_ray(Ray ray,
                                            float max_distance) const = 0;
};
