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

class World {
public:
  World();
  ~World();
  // The world currently has no time-based term, but the dt parameter is kept
  // so a future animated-chunk step slots in without touching callers.
  void update(float, Vector3);
  void draw() const;
  bool position_is_acceptable(Vector3) const;
  bool is_ceiling_blocked(Vector3) const;
  bool is_solid(Vector3) const;
  float get_floor_height(float, float) const;
  Vector3 find_blink_target(Vector3 origin, Vector3 direction,
                            float max_dist) const;
  Vector3 find_blink_target_through(Vector3 origin, Vector3 direction,
                                    float max_dist) const;
  bool has_line_of_sight(Vector3 from, Vector3 to) const;
  void set_renderer(Renderer *);
  void build_chunks();

private:
  static constexpr int kChunkSize = 64;
  static constexpr int kWorldSizeChunks = 16;     // length of nxn chunk world
  static constexpr int kRenderDistanceChunks = 6; // in chunks

  std::vector<std::unique_ptr<Chunk>> chunks_;
  Renderer *renderer_;

  // Threading for chunk loading
  std::thread loading_thread_;
  std::mutex chunks_mutex_;
  std::mutex load_queue_mutex_;
  std::condition_variable load_cv_;
  std::queue<Chunk *> load_queue_;
  std::atomic<bool> should_exit_;

  // Shared coordinate math — every world↔chunk conversion goes through these.
  struct ChunkCoord {
    int cx;
    int cz;
    int lx;
    int lz;
  };
  ChunkCoord world_to_chunk(float x, float z) const;
  Chunk *find_chunk(int cx, int cz) const;
  int chunk_dist2(const Chunk &chunk, int player_chunk_x,
                  int player_chunk_z) const;

  bool load_chunk_data(const std::string &);
  Chunk *get_or_create_chunk(int, int);
  void chunk_loading_worker();
  void update_chunk_loading(Vector3 player_position);
  void upload_one_ready_mesh();
  void unload_far_chunks(int player_chunk_x, int player_chunk_z);
  void queue_nearby_chunks(int player_chunk_x, int player_chunk_z);
  void make_random_pillars(Chunk *);

  // Blink raymarch sub-checks — the order they are called in is gameplay.
  bool pillar_top_lock(const Vector3 &candidate, float floor_cam_y,
                       float player_floor_y, Vector3 &lock) const;
  void hug_elevated_floor(Vector3 &candidate, float floor_cam_y,
                          float player_floor_y) const;
  void surface_pop(const Vector3 &candidate, float floor_cam_y,
                   Vector3 &last_valid) const;
  void snap_to_ledge(Vector3 &last_valid, float player_floor_y) const;
};
