#include "world.h"
#include "raymath.h"
#include "utils/random.h"
#include <cctype>
#include <cmath>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <thread>

namespace {

// ── Blink raymarch and collision-probe tuning — values are gameplay ──
constexpr float kBlinkStep = 0.1f;      // raymarch sample spacing
constexpr float kBlinkStartDist = 0.8f; // first sample just off the camera
constexpr float kEyeHeight = 2.0f;      // camera height above the feet
constexpr float kBlinkElevatedDelta =
    0.5f; // "elevated" threshold above the player floor
constexpr float kBlinkTopBand = 1.0f; // top of the pillar-top lock band
constexpr float kBlinkPopMaxDrop =
    4.0f; // max floor→ray gap that may pop to surface
constexpr float kBlinkSnapRange = 1.5f; // magnetic-snap distance to a ledge top
constexpr float kPlayerRadius = 0.35f;  // collision half-width
constexpr float kCeilingProbeRadius = 0.15f; // head-bump probe half-width
constexpr float kFootClearance = 0.05f;      // ground tolerance above the feet
constexpr float kLosStep = 0.5f;             // line-of-sight sample spacing
constexpr float kLosMinDist = 0.01f; // below this the endpoints count as clear

// ── Zone selection ──
constexpr float kZoneSpireMax = 0.33f; // noise below this → spire zone
constexpr float kZoneMixedMax = 0.66f; // noise below this → mixed zone
constexpr float kIntensityRampChunks = 5.0f;
constexpr float kSpireWideIntensity = 0.3f;

// Start-area override: guarantees visual density near spawn regardless of zone.
constexpr float kStartAreaRadius = 2.0f;
constexpr int kStartAreaMinAttempts = 60;
constexpr int kStartAreaMinHeight = 12;
constexpr int kStartAreaBaseHeight = 3;
constexpr int kStartAreaMinWidth = 4;

constexpr int kMaxInlineChunkMeshes = 2; // web inline mesh generation per frame
constexpr int kFloorHeight = 1;

// Standard value-noise hash primes — kept inline by convention, not tuning.
float hash2d(int x, int z) {
  uint32_t h = (uint32_t)(x * 1619 + z * 31337);
  h ^= h >> 16;
  h *= 0x45d9f3b;
  h ^= h >> 16;
  return (float)(h & 0xFFFF) / 65535.0f;
}

// Control-grid step: 5/7 of a chunk. Held as num/den so the
// multiply-then-divide rounding order — and therefore the zone boundaries —
// stays bit-identical.
constexpr float kNoiseScaleNum = 5.0f;
constexpr float kNoiseScaleDen = 7.0f;

// Bilinear interpolation over a fine control grid — zone transitions happen
// within roughly 1 chunk of walking.
float smooth_noise(int cx, int cz) {
  float u = cx * kNoiseScaleNum / kNoiseScaleDen;
  float v = cz * kNoiseScaleNum / kNoiseScaleDen;
  int ix = (int)u;
  float fx = u - ix;
  int iz = (int)v;
  float fz = v - iz;
  float c00 = hash2d(ix, iz);
  float c10 = hash2d(ix + 1, iz);
  float c01 = hash2d(ix, iz + 1);
  float c11 = hash2d(ix + 1, iz + 1);
  return (c00 * (1 - fx) + c10 * fx) * (1 - fz) +
         (c01 * (1 - fx) + c11 * fx) * fz;
}

// Per-zone pillar tuning. The instances carry the fixed per-zone numbers
// verbatim; fields that ramp with distance are applied in select_zone_params.
// Values are gameplay — never retune here.
struct PillarZoneParams {
  int num_attempts;
  int h_min;
  int h_max;
  int w_min;
  int w_max;
  int probability;
};
constexpr PillarZoneParams kSpireZone{28, 6, 16, 1, 2, 72};
constexpr PillarZoneParams kMixedZone{50, 2, 10, 2, 4, 75};
constexpr PillarZoneParams kRubbleZone{65, 1, 5, 3, 6, 88};

// noise → zone type; intensity → extremeness, ramps up quickly with distance
PillarZoneParams select_zone_params(float noise, float intensity) {
  PillarZoneParams p;
  if (noise < kZoneSpireMax) {
    // Spire zone: very tall, very narrow, sparse — cathedral-like far out
    p = kSpireZone;
    p.num_attempts = (int)(28.0f - 16.0f * intensity); // 28 →  12
    p.h_min = (int)(6.0f + 12.0f * intensity);         //  6 →  18
    p.h_max = (int)(16.0f + 16.0f * intensity);        // 16 →  32
    p.w_max = intensity > kSpireWideIntensity ? 1 : 2;
  } else if (noise < kZoneMixedMax) {
    // Mixed zone: solid variety, grows taller and denser with distance
    p = kMixedZone;
    p.num_attempts = (int)(50.0f + 20.0f * intensity); // 50 →  70
    p.h_max = (int)(10.0f + 12.0f * intensity);        // 10 →  22
    p.w_max = (int)(4.0f + 2.0f * intensity);          //  4 →   6
  } else {
    // Rubble zone: short, wide, very dense — almost maze-like far out
    p = kRubbleZone;
    p.num_attempts = (int)(65.0f + 45.0f * intensity); // 65 → 110
    p.h_max = (int)(5.0f - 2.0f * intensity);          //  5 →   3
    p.w_min = (int)(3.0f + 2.0f * intensity);          //  3 →   5
    p.w_max = (int)(6.0f + 5.0f * intensity);          //  6 →  11
  }
  return p;
}

// Guarantee visual density near the starting area regardless of zone type
void apply_start_area_override(float dist, PillarZoneParams &p) {
  if (dist >= kStartAreaRadius)
    return;
  p.num_attempts = std::max(p.num_attempts, kStartAreaMinAttempts);
  p.h_max = std::max(p.h_max, kStartAreaMinHeight);
  p.h_min = std::min(p.h_min, kStartAreaBaseHeight);
  p.w_max = std::max(p.w_max, kStartAreaMinWidth);
}

bool is_all_digits(const std::string &s) {
  return !s.empty() && std::all_of(s.begin(), s.end(), [](unsigned char c) {
    return std::isdigit(c);
  });
}

std::vector<std::string> split_whitespace(const std::string &s) {
  std::istringstream iss(s);
  std::vector<std::string> tokens;
  std::string token;

  while (iss >> token) { // operator>> automatically skips whitespace
    tokens.push_back(token);
  }

  return tokens;
}

void add_pillar(Chunk *chunk, int x, int z, int height) {
  for (int y = 0; y < height; ++y) {
    chunk->set_voxel(x, y, z);
  }
}

// floor
void fill_chunk_floor(Chunk *chunk, int chunk_size) {
  for (int x = 0; x < chunk_size; ++x)
    for (int z = 0; z < chunk_size; ++z)
      add_pillar(chunk, x, z, kFloorHeight);
}

// pillars
void scatter_pillars(Chunk *chunk, const PillarZoneParams &p, int chunk_size) {
  RandomNumberGenerator<int> random_height(p.h_min, p.h_max);
  RandomNumberGenerator<int> random_size(p.w_min, p.w_max);
  RandomNumberGenerator<int> random_pos(0, chunk_size - 1);
  RandomNumberGenerator<int> is_pillar(0, 100);

  for (int n = 0; n < p.num_attempts; ++n) {
    if (is_pillar() >= p.probability)
      continue;

    int x = random_pos();
    int z = random_pos();
    int width = random_size();
    int height = random_height();
    int max_x = std::min(x + width, chunk_size);
    int max_z = std::min(z + width, chunk_size);

    for (int dx = 0; dx < max_x - x; ++dx)
      for (int dz = 0; dz < max_z - z; ++dz)
        add_pillar(chunk, x + dx, z + dz, height);
  }
}

} // namespace

void World::make_random_pillars(Chunk *chunk) {
  int cx = (int)chunk->get_position().x;
  int cz = (int)chunk->get_position().y;

  // noise → zone type; intensity → extremeness, ramps up quickly with distance
  float noise = smooth_noise(cx, cz);
  float dist = sqrtf((float)(cx * cx + cz * cz));
  float intensity = std::min(dist / kIntensityRampChunks,
                             1.0f); // full intensity by ~5 chunks out

  PillarZoneParams p = select_zone_params(noise, intensity);

  p.h_max = std::max(p.h_max, p.h_min + 1);
  p.w_max = std::max(p.w_max, p.w_min);

  apply_start_area_override(dist, p);

  fill_chunk_floor(chunk, kChunkSize);
  scatter_pillars(chunk, p, kChunkSize);
}

World::World() : should_exit_(false) {
#ifndef PLATFORM_WEB
  // Background chunk loader (desktop only — pthreads need COOP/COEP headers
  // that static hosting like GitHub Pages cannot send).
  loading_thread_ = std::thread(&World::chunk_loading_worker, this);
#endif
}

void World::build_chunks() {
  load_chunk_data("maps/home.txt");
  for (int i = 0; i < kWorldSizeChunks; ++i) {
    for (int j = 0; j < kWorldSizeChunks; ++j) {
      // we hard-coded chunk 0,0
      if (i == 0 && j == 0)
        continue;
      Chunk *current_chunk = get_or_create_chunk(i, j);
      // fill the chunk with random pillars
      make_random_pillars(current_chunk);
    }
    // Lighting rig (sun + fill + purple accent) lives in Renderer.
  }
}

World::~World() {
  // Signal the thread to exit
  should_exit_ = true;
  load_cv_.notify_one();
  if (loading_thread_.joinable())
    loading_thread_.join();
}

void World::set_renderer(Renderer *renderer) { renderer_ = renderer; }

// Shared world→chunk coordinate math: floor-divide to chunk indices, then
// floor-mod to in-chunk local coordinates.
World::ChunkCoord World::world_to_chunk(float x, float z) const {
  ChunkCoord coord;
  coord.cx = (int)std::floor(x / kChunkSize);
  coord.cz = (int)std::floor(z / kChunkSize);
  coord.lx = (int)std::floor(x) - coord.cx * kChunkSize;
  coord.lz = (int)std::floor(z) - coord.cz * kChunkSize;
  return coord;
}

// Linear scan for the chunk at (cx, cz); nullptr when there is none.
Chunk *World::find_chunk(int cx, int cz) const {
  for (auto &c : chunks_) {
    if ((int)c->get_position().x == cx && (int)c->get_position().y == cz)
      return c.get();
  }
  return nullptr;
}

// Squared chunk-space distance from a chunk to the player's chunk.
int World::chunk_dist2(const Chunk &chunk, int player_chunk_x,
                       int player_chunk_z) const {
  int dx = (int)chunk.get_position().x - player_chunk_x;
  int dy = (int)chunk.get_position().y - player_chunk_z;
  return dx * dx + dy * dy;
}

Chunk *World::get_or_create_chunk(int cx, int cy) {
  if (Chunk *existing = find_chunk(cx, cy))
    return existing;
  // Not found → create new
  auto chunk = std::make_unique<Chunk>(Vector2{float(cx), float(cy)},
                                       kChunkSize, renderer_);
  Chunk *ptr = chunk.get();
  chunks_.push_back(std::move(chunk));
  return ptr;
}

bool World::load_chunk_data(const std::string &filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Failed to open file: " << filename << "\n";
    return false;
  }

  Chunk *current_chunk = nullptr;
  int current_chunk_x = 0;
  int current_chunk_y = 0;

  int line_num = 0;
  std::string line;

  while (std::getline(file, line)) {
    line_num++;

    // Trim leading whitespace
    size_t start = line.find_first_not_of(" \t");
    if (start == std::string::npos)
      continue; // Empty line
    line = line.substr(start);

    // Skip comments and empty lines
    if (line.empty() || line[0] == '#')
      continue;

    std::istringstream ss(line);
    std::string command;
    ss >> command;

    if (command == "chunk") {
      int cx, cy;
      if (!(ss >> cx >> cy)) {
        std::cerr << "Line " << line_num << ": Invalid chunk command\n";
        continue;
      }
      current_chunk = get_or_create_chunk(cx, cy);
      current_chunk_x = 0;
      current_chunk_y = 0;
    } else if (is_all_digits(command)) {
      std::vector<std::string> tokens = split_whitespace(line);
      for (auto &token : tokens) {
        int height = std::stoi(token);
        // always add a "pillar" (if value is zero, add a single block as a
        // floor)
        add_pillar(current_chunk, current_chunk_x, current_chunk_y,
                   height + kFloorHeight);
        current_chunk_x++;
      }
      current_chunk_x = 0;
      current_chunk_y++;
    } else {
      std::cerr << "Line " << line_num << ": Unknown command '" << command
                << "'\n";
    }
  }

  return true;
}

void World::update(float, Vector3 current_player_position) {
  update_chunk_loading(current_player_position);
}

void World::draw() const {
  for (auto &c : chunks_) {
    if (c->state == ChunkState::LOADED) {
      c->draw();
    }
  }
}

void World::chunk_loading_worker() {
  while (!should_exit_) {
    Chunk *chunk = nullptr;
    {
      std::unique_lock<std::mutex> lock(load_queue_mutex_);
      load_cv_.wait(
          lock, [this] { return !load_queue_.empty() || should_exit_.load(); });
      if (should_exit_)
        break;
      chunk = load_queue_.front();
      load_queue_.pop();
    }
    chunk->generate_mesh();
  }
}

// All three passes below require chunks_mutex_ to be held.
void World::update_chunk_loading(Vector3 player_position) {
  // Convert player position to chunk coordinates
  ChunkCoord player = world_to_chunk(player_position.x, player_position.z);

  std::lock_guard<std::mutex> lock(chunks_mutex_);

  upload_one_ready_mesh();
  unload_far_chunks(player.cx, player.cz);
  queue_nearby_chunks(player.cx, player.cz);
}

// Upload one chunk per frame to avoid GPU stalls blocking input polling
void World::upload_one_ready_mesh() {
  for (auto &chunk : chunks_) {
    if (chunk->state == ChunkState::READY_TO_UPLOAD) {
      chunk->upload_mesh();
      break;
    }
  }
}

// Unload chunks that are too far away
void World::unload_far_chunks(int player_chunk_x, int player_chunk_z) {
  const int rd2 = kRenderDistanceChunks * kRenderDistanceChunks;
  for (auto &chunk : chunks_) {
    if (chunk_dist2(*chunk, player_chunk_x, player_chunk_z) > rd2 &&
        chunk->state == ChunkState::LOADED)
      chunk->unload();
  }
}

// Queue chunks for loading that are within range. On web there is no
// background thread, so meshes generate inline — cap it per frame to keep
// the main thread responsive.
void World::queue_nearby_chunks(int player_chunk_x, int player_chunk_z) {
  const int rd2 = kRenderDistanceChunks * kRenderDistanceChunks;
  int generated_this_frame = 0;
  for (auto &chunk : chunks_) {
    if (chunk_dist2(*chunk, player_chunk_x, player_chunk_z) <= rd2 &&
        chunk->state == ChunkState::UNLOADED) {
      chunk->state = ChunkState::GENERATING;
#ifndef PLATFORM_WEB
      {
        std::lock_guard<std::mutex> queue_lock(load_queue_mutex_);
        load_queue_.push(chunk.get());
      }
      load_cv_.notify_one();
#else
      chunk->generate_mesh();
      if (++generated_this_frame >= kMaxInlineChunkMeshes)
        break; // spread the work; the rest load over the next frames
#endif
    }
  }
}

bool World::is_solid(Vector3 pos) const {
  int vy = (int)std::floor(pos.y);
  if (vy < 0)
    return false;
  ChunkCoord coord = world_to_chunk(pos.x, pos.z);
  Chunk *chunk = find_chunk(coord.cx, coord.cz);
  return chunk && chunk->is_filled(coord.lx, vy, coord.lz);
}

Vector3 World::find_blink_target(Vector3 origin, Vector3 direction,
                                 float max_dist) const {
  float player_floor_y = get_floor_height(origin.x, origin.z) + kEyeHeight;
  Vector3 last_valid = origin;

  for (float dist = kBlinkStartDist; dist <= max_dist; dist += kBlinkStep) {
    Vector3 candidate = Vector3Add(origin, Vector3Scale(direction, dist));
    float floor_cam_y = get_floor_height(candidate.x, candidate.z) + kEyeHeight;

    Vector3 lock;
    if (pillar_top_lock(candidate, floor_cam_y, player_floor_y, lock))
      return lock;

    hug_elevated_floor(candidate, floor_cam_y, player_floor_y);

    if (is_solid(candidate)) {
      surface_pop(candidate, floor_cam_y, last_valid);
      break;
    }
    if (position_is_acceptable(candidate))
      last_valid = candidate;
  }

  snap_to_ledge(last_valid, player_floor_y);

  return last_valid;
}

// Hard rule: if the raw ray (before any hugging) is passing through the
// vertical band of an elevated surface top, the player is looking directly
// at a pillar top — lock there immediately and show the cone.
bool World::pillar_top_lock(const Vector3 &candidate, float floor_cam_y,
                            float player_floor_y, Vector3 &lock) const {
  if (floor_cam_y > player_floor_y + kBlinkElevatedDelta &&
      candidate.y >= floor_cam_y - kBlinkElevatedDelta &&
      candidate.y <= floor_cam_y + kBlinkTopBand) {
    Vector3 top = {candidate.x, floor_cam_y, candidate.z};
    if (position_is_acceptable(top)) {
      lock = top;
      return true;
    }
  }
  return false;
}

// Only hug elevated surfaces — when the target floor is HIGHER than the
// player's start floor (aiming at a ledge/pillar top). Skip hugging when
// looking down to lower or equal ground so the ball tracks the crosshair.
void World::hug_elevated_floor(Vector3 &candidate, float floor_cam_y,
                               float player_floor_y) const {
  if (candidate.y < floor_cam_y && floor_cam_y > player_floor_y)
    candidate.y = floor_cam_y;
}

// Pop to surface top only if it's within a reasonable vertical range —
// prevents jarring jumps when a tall pillar is barely grazed by the ray.
void World::surface_pop(const Vector3 &candidate, float floor_cam_y,
                        Vector3 &last_valid) const {
  if (floor_cam_y - candidate.y < kBlinkPopMaxDrop) {
    Vector3 surface_top = {candidate.x, floor_cam_y, candidate.z};
    if (position_is_acceptable(surface_top))
      last_valid = surface_top;
  }
}

// Magnetic snap: if last_valid lands within 1.5 units of an elevated surface
// top, lock to it — makes it easy to intentionally land on ledges.
void World::snap_to_ledge(Vector3 &last_valid, float player_floor_y) const {
  float last_floor = get_floor_height(last_valid.x, last_valid.z) + kEyeHeight;
  if (last_floor > player_floor_y + kBlinkElevatedDelta &&
      fabsf(last_valid.y - last_floor) < kBlinkSnapRange) {
    Vector3 snapped = {last_valid.x, last_floor, last_valid.z};
    if (position_is_acceptable(snapped))
      last_valid = snapped;
  }
}

// Tap blink variant: scans the full range without stopping at obstacles,
// taking the furthest valid position. Lets the player zip through walls.
Vector3 World::find_blink_target_through(Vector3 origin, Vector3 direction,
                                         float max_dist) const {
  Vector3 last_valid = origin;
  for (float dist = kBlinkStartDist; dist <= max_dist; dist += kBlinkStep) {
    Vector3 candidate = Vector3Add(origin, Vector3Scale(direction, dist));
    float floor_cam_y = get_floor_height(candidate.x, candidate.z) + kEyeHeight;
    if (candidate.y < floor_cam_y)
      candidate.y = floor_cam_y;
    if (position_is_acceptable(candidate))
      last_valid = candidate;
    // no break — keep scanning through obstacles
  }
  return last_valid;
}

bool World::is_ceiling_blocked(Vector3 camera_pos) const {
  // Narrow radius — wall clearance (kPlayerRadius) keeps us away from wall
  // faces, so this only fires for voxels genuinely above the player's head
  float head_y = camera_pos.y;
  float xs[] = {camera_pos.x - kCeilingProbeRadius,
                camera_pos.x + kCeilingProbeRadius};
  float zs[] = {camera_pos.z - kCeilingProbeRadius,
                camera_pos.z + kCeilingProbeRadius};
  for (float x : xs)
    for (float z : zs)
      if (is_solid({x, head_y, z}))
        return true;
  return false;
}

float World::get_floor_height(float x, float z) const {
  // The per-chunk column-height lookup is O(1) (was a 64-step voxel scan),
  // but finding the owning chunk is still a linear scan over chunks_.
  ChunkCoord coord = world_to_chunk(x, z);
  Chunk *chunk = find_chunk(coord.cx, coord.cz);
  if (!chunk)
    return 0.0f;
  return (float)chunk->column_height(coord.lx, coord.lz);
}

bool World::position_is_acceptable(Vector3 camera_pos) const {
  float feet_y = camera_pos.y - kEyeHeight;

  float ys[] = {feet_y + kFootClearance, camera_pos.y - kEyeHeight * 0.5f,
                camera_pos.y};
  float xs[] = {camera_pos.x - kPlayerRadius, camera_pos.x + kPlayerRadius};
  float zs[] = {camera_pos.z - kPlayerRadius, camera_pos.z + kPlayerRadius};

  for (float y : ys)
    for (float x : xs)
      for (float z : zs)
        if (is_solid({x, y, z}))
          return false;

  return true;
}

bool World::has_line_of_sight(Vector3 from, Vector3 to) const {
  Vector3 delta = Vector3Subtract(to, from);
  float dist = Vector3Length(delta);
  if (dist < kLosMinDist)
    return true;
  int steps = (int)(dist / kLosStep);
  for (int i = 1; i < steps; ++i) {
    float t = (float)i * kLosStep / dist;
    Vector3 p = Vector3Add(from, Vector3Scale(delta, t));
    if (is_solid(p))
      return false;
  }
  return true;
}
