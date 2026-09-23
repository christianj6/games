#pragma once
#include "raylib.h"
#include "utils/graphics/renderer.h"
#include <atomic>
#include <mutex>
#include <vector>

enum class ChunkState { UNLOADED, GENERATING, READY_TO_UPLOAD, LOADED };

struct MeshData {
  std::vector<float> vertices;
  std::vector<float> normals;
  std::vector<float> texcoords;
  std::vector<unsigned short> indices;
};

class Chunk {
public:
  Chunk(Vector2 position, int size, Renderer *renderer);
  ~Chunk();

  void generate_mesh();
  void upload_mesh();
  void unload();
  void draw();

  void set_voxel(int x, int y, int z);
  void clear_voxel(int x, int y, int z);
  bool is_filled(int x, int y, int z) const;
  // Topmost solid voxel + 1 for a column (0 = empty). O(1) via cache.
  int column_height(int lx, int lz) const { return heights_[lx * size_ + lz]; }

  Vector2 get_position() const { return position_; }

  std::atomic<ChunkState> state{ChunkState::UNLOADED};

private:
  Vector2 position_;
  int size_;
  std::vector<uint8_t> voxels_;
  std::vector<uint8_t> heights_; // per-column top surface, kept by set/clear
  Mesh mesh_;
  MeshData mesh_data_;
  std::mutex mesh_data_mutex_;

  Material material_;
  bool is_in_bounds(int x, int y, int z) const;
  void build_slice_mask(std::vector<bool> &mask, int dim, int u, int v,
                        int side, int slice) const;
  static void compute_quad_corners(float p[4][3], int dim, int u, int v,
                                   int side, int slice, int i, int j, int w,
                                   int h);
};
