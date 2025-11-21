#pragma once
#include "chunk.h"
#include "raylib.h"

#include <memory>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>

struct ChunkLoadRequest {
  int chunk_x;
  int chunk_y;
  Chunk* chunk_ptr;
};

class World {
public:
  World();
  ~World();
  void update(float, Vector3);
  void draw();
  bool position_is_acceptable(const Vector3);

private:
  int chunk_size_;
  int render_distance_;  // in chunks
  std::vector<std::unique_ptr<Chunk>> chunks_;

  // Threading for chunk loading
  std::thread loading_thread_;
  std::mutex chunks_mutex_;
  std::mutex load_queue_mutex_;
  std::queue<ChunkLoadRequest> load_queue_;
  std::atomic<bool> should_exit_;

  bool load_chunk_data(const std::string&);
  Chunk* get_or_create_chunk(int, int);
  void chunk_loading_worker();
  void update_chunk_loading(Vector3 player_position);

  // Shape primitive helpers
  void add_box(Chunk*, int x1, int y1, int z1, int x2, int y2, int z2);
  void add_hollow_box(Chunk*, int x1, int y1, int z1, int x2, int y2, int z2);
  void add_floor(Chunk*, int x1, int z1, int x2, int z2, int y = 0);
  void add_pillar(Chunk*, int x, int z, int y1, int y2);
  void add_wall_x(Chunk*, int z, int y1, int y2, int x1, int x2);
  void add_wall_z(Chunk*, int x, int y1, int y2, int z1, int z2);
};
