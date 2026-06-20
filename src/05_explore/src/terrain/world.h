#pragma once
#include "chunk.h"
#include "raylib.h"
#include "utils/graphics/renderer.h"

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

struct ChunkLoadRequest {
  int chunk_x;
  int chunk_y;
  Chunk *chunk_ptr;
};

class World {
public:
  World();
  ~World();
  void update(float, Vector3);
  void draw();
  bool position_is_acceptable(Vector3) const;
  bool is_ceiling_blocked(Vector3) const;
  float get_floor_height(float, float) const;
  Vector3 find_blink_target(Vector3 origin, Vector3 direction, float max_dist) const;
  Vector3 find_blink_target_through(Vector3 origin, Vector3 direction, float max_dist) const;
  void set_renderer(Renderer *);
  void build_chunks();

private:
  int chunk_size_;
  int world_size_chunks_; // length of nxn chunk world
  int render_distance_;   // in chunks
  std::vector<std::unique_ptr<Chunk>> chunks_;
  Renderer *renderer_;

  // Threading for chunk loading
  std::thread loading_thread_;
  std::mutex chunks_mutex_;
  std::mutex load_queue_mutex_;
  std::condition_variable load_cv_;
  std::queue<ChunkLoadRequest> load_queue_;
  std::atomic<bool> should_exit_;

  bool load_chunk_data(const std::string &);
  Chunk *get_or_create_chunk(int, int);
  void chunk_loading_worker();
  void update_chunk_loading(Vector3 player_position);
  void make_random_pillars(Chunk *);

  void add_pillar(Chunk *, int, int, int);
  bool is_solid(Vector3) const;
};
