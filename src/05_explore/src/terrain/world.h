#pragma once
#include "chunk.h"
#include "raylib.h"

#include <memory>
#include <vector>
#include <string>

class World {
public:
  World();
  void update(float, Vector3);
  void draw();
  bool position_is_acceptable(const Vector3);

private:
  int chunk_size_;
  std::vector<std::unique_ptr<Chunk>> chunks_;

  bool load_chunk_data(const std::string&);
  Chunk* get_or_create_chunk(int, int);
};
