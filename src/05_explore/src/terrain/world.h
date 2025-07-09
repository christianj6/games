#pragma once
#include "chunk.h"
#include "raylib.h"

#include <memory>
#include <vector>

class World {
public:
  World();
  void update(float, Vector3);
  void draw();
  bool position_is_acceptable(const Vector3);

private:
  std::vector<std::unique_ptr<Chunk>> chunks;
};
