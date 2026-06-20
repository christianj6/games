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
  Chunk(Vector2, int = 32, Renderer * = nullptr);
  ~Chunk();

  void generate_mesh(); // CPU-intensive, can run on worker thread
  void upload_mesh();   // Must run on main thread (OpenGL)
  void unload();
  void draw();

  void set_voxel(int, int, int);
  void clear_voxel(int, int, int);
  bool is_filled(int, int, int) const;

  Vector2 get_position() { return position_; }

  std::atomic<ChunkState> state{ChunkState::UNLOADED};
  bool loaded = false;

private:
  Vector2 position_;
  int size_;
  std::vector<uint8_t> voxels_; // flat [x*size*size + y*size + z], 1=filled
  Mesh mesh_;
  MeshData mesh_data_;
  std::mutex mesh_data_mutex_;

  Material material_;
  bool is_in_bounds(int, int, int) const;
};
