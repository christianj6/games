#pragma once
#include "raylib.h"
#include <unordered_set>

class Chunk {
public:
  Chunk(Vector2, int = 32);
  ~Chunk();

  void load();
  void unload();
  void draw();

  void set_voxel(int, int, int);
  void clear_voxel(int, int, int);
  bool is_filled(int, int, int) const;

  Vector2 get_position() {return position_;}

  bool loaded = false;

private:
  Vector2 position_;
  int size_;
  std::unordered_set<uint32_t> voxels_;
  Mesh mesh_;

  bool is_in_bounds(int, int, int) const;
};
