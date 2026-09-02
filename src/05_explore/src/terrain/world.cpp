#include "world.h"
#include "raylib.h"
#include "raymath.h"
#include "utils/random.h"
#include <cctype>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <thread>

static float hash2d(int x, int z) {
  uint32_t h = (uint32_t)(x * 1619 + z * 31337);
  h ^= h >> 16;
  h *= 0x45d9f3b;
  h ^= h >> 16;
  return (float)(h & 0xFFFF) / 65535.0f;
}

// Bilinear interpolation over a fine control grid — zone transitions happen
// within roughly 1 chunk of walking.
static float smooth_noise(int cx, int cz) {
  float u = cx * 5.0f / 7.0f;
  float v = cz * 5.0f / 7.0f;
  int ix = (int)u; float fx = u - ix;
  int iz = (int)v; float fz = v - iz;
  float c00 = hash2d(ix,     iz);
  float c10 = hash2d(ix + 1, iz);
  float c01 = hash2d(ix,     iz + 1);
  float c11 = hash2d(ix + 1, iz + 1);
  return (c00 * (1 - fx) + c10 * fx) * (1 - fz) +
         (c01 * (1 - fx) + c11 * fx) * fz;
}

void World::make_random_pillars(Chunk *chunk) {
  int cx = (int)chunk->get_position().x;
  int cz = (int)chunk->get_position().y;

  // noise → zone type; intensity → extremeness, ramps up quickly with distance
  float noise     = smooth_noise(cx, cz);
  float dist      = sqrtf((float)(cx * cx + cz * cz));
  float intensity = std::min(dist / 5.0f, 1.0f); // full intensity by ~5 chunks out

  int num_attempts, h_min, h_max, w_min, w_max, probability;

  if (noise < 0.33f) {
    // Spire zone: very tall, very narrow, sparse — cathedral-like far out
    num_attempts = (int)(28.0f - 16.0f * intensity); // 28 →  12
    h_min        = (int)( 6.0f + 12.0f * intensity); //  6 →  18
    h_max        = (int)(16.0f + 16.0f * intensity); // 16 →  32
    w_min        = 1;
    w_max        = intensity > 0.3f ? 1 : 2;
    probability  = 72;
  } else if (noise < 0.66f) {
    // Mixed zone: solid variety, grows taller and denser with distance
    num_attempts = (int)(50.0f + 20.0f * intensity); // 50 →  70
    h_min        = 2;
    h_max        = (int)(10.0f + 12.0f * intensity); // 10 →  22
    w_min        = 2;
    w_max        = (int)( 4.0f +  2.0f * intensity); //  4 →   6
    probability  = 75;
  } else {
    // Rubble zone: short, wide, very dense — almost maze-like far out
    num_attempts = (int)(65.0f + 45.0f * intensity); // 65 → 110
    h_min        = 1;
    h_max        = (int)( 5.0f -  2.0f * intensity); //  5 →   3
    w_min        = (int)( 3.0f +  2.0f * intensity); //  3 →   5
    w_max        = (int)( 6.0f +  5.0f * intensity); //  6 →  11
    probability  = 88;
  }

  h_max = std::max(h_max, h_min + 1);
  w_max = std::max(w_max, w_min);

  // Guarantee visual density near the starting area regardless of zone type
  if (dist < 2.0f) {
    num_attempts = std::max(num_attempts, 60);
    h_max        = std::max(h_max, 12);
    h_min        = std::min(h_min, 3);
    w_max        = std::max(w_max, 4);
  }

  RandomNumberGenerator<int> random_height(h_min, h_max);
  RandomNumberGenerator<int> random_size(w_min, w_max);
  RandomNumberGenerator<int> random_pos(0, chunk_size_ - 1);
  RandomNumberGenerator<int> is_pillar(0, 100);

  // floor
  for (int x = 0; x < chunk_size_; ++x)
    for (int z = 0; z < chunk_size_; ++z)
      add_pillar(chunk, x, z, 1);

  // pillars
  for (int n = 0; n < num_attempts; ++n) {
    if (is_pillar() >= probability)
      continue;

    int x      = random_pos();
    int z      = random_pos();
    int width  = random_size();
    int height = random_height();
    int max_x  = std::min(x + width, chunk_size_);
    int max_z  = std::min(z + width, chunk_size_);

    for (int dx = 0; dx < max_x - x; ++dx)
      for (int dz = 0; dz < max_z - z; ++dz)
        add_pillar(chunk, x + dx, z + dz, height);
  }
}

World::World() : should_exit_(false) {
  chunk_size_ = 64;
  world_size_chunks_ = 16;
  render_distance_ = 6;

  // Start the chunk loading thread
  loading_thread_ = std::thread(&World::chunk_loading_worker, this);
}

void World::build_chunks() {
  load_chunk_data("maps/home.txt");
  for (int i = 0; i < world_size_chunks_; ++i) {
    for (int j = 0; j < world_size_chunks_; ++j) {
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

Chunk *World::get_or_create_chunk(int cx, int cy) {
  for (auto &c : chunks_) {
    if (c->get_position().x == cx && c->get_position().y == cy)
      return c.get();
  }
  // Not found → create new
  auto chunk = std::make_unique<Chunk>(Vector2{float(cx), float(cy)},
                                       chunk_size_, renderer_);
  Chunk *ptr = chunk.get();
  chunks_.push_back(std::move(chunk));
  return ptr;
}

void World::add_pillar(Chunk *chunk, int x, int z, int height) {
  for (int y = 0; y < height; ++y) {
    chunk->set_voxel(x, y, z);
  }
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
        add_pillar(current_chunk, current_chunk_x, current_chunk_y, height + 1);
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

void World::update(float dt, Vector3 current_player_position) {
  update_chunk_loading(current_player_position);
}

void World::draw() {
  for (auto &c : chunks_) {
    if (c->loaded) {
      c->draw();
    }
  }
}

void World::chunk_loading_worker() {
  while (!should_exit_) {
    ChunkLoadRequest request;
    {
      std::unique_lock<std::mutex> lock(load_queue_mutex_);
      load_cv_.wait(lock, [this] {
        return !load_queue_.empty() || should_exit_.load();
      });
      if (should_exit_)
        break;
      request = load_queue_.front();
      load_queue_.pop();
    }
    request.chunk_ptr->generate_mesh();
  }
}

void World::update_chunk_loading(Vector3 player_position) {
  // Convert player position to chunk coordinates
  int player_chunk_x =
      static_cast<int>(std::floor(player_position.x / chunk_size_));
  int player_chunk_z =
      static_cast<int>(std::floor(player_position.z / chunk_size_));

  std::lock_guard<std::mutex> lock(chunks_mutex_);

  // Upload one chunk per frame to avoid GPU stalls blocking input polling
  for (auto &chunk : chunks_) {
    if (chunk->state == ChunkState::READY_TO_UPLOAD) {
      chunk->upload_mesh();
      break;
    }
  }

  // Unload chunks that are too far away
  for (auto &chunk : chunks_) {
    int chunk_x = static_cast<int>(chunk->get_position().x);
    int chunk_y = static_cast<int>(chunk->get_position().y);

    int dx = chunk_x - player_chunk_x;
    int dy = chunk_y - player_chunk_z;
    int dist2 = dx * dx + dy * dy;
    int rd2 = render_distance_ * render_distance_;

    if (dist2 > rd2 && chunk->state == ChunkState::LOADED)
      chunk->unload();
  }

  // Queue chunks for loading that are within range
  for (auto &chunk : chunks_) {
    int chunk_x = static_cast<int>(chunk->get_position().x);
    int chunk_y = static_cast<int>(chunk->get_position().y);

    int dx = chunk_x - player_chunk_x;
    int dy = chunk_y - player_chunk_z;
    int dist2 = dx * dx + dy * dy;
    int rd2 = render_distance_ * render_distance_;

    if (dist2 <= rd2 && chunk->state == ChunkState::UNLOADED) {
      chunk->state = ChunkState::GENERATING;
      {
        std::lock_guard<std::mutex> queue_lock(load_queue_mutex_);
        load_queue_.push({chunk_x, chunk_y, chunk.get()});
      }
      load_cv_.notify_one();
    }
  }
}

bool World::is_solid(Vector3 pos) const {
  int vy = (int)std::floor(pos.y);
  if (vy < 0)
    return false;
  int cx = (int)std::floor(pos.x / chunk_size_);
  int cz = (int)std::floor(pos.z / chunk_size_);
  int lx = (int)std::floor(pos.x) - cx * chunk_size_;
  int lz = (int)std::floor(pos.z) - cz * chunk_size_;
  for (auto &c : chunks_) {
    if ((int)c->get_position().x == cx && (int)c->get_position().y == cz)
      return c->is_filled(lx, vy, lz);
  }
  return false;
}

Vector3 World::find_blink_target(Vector3 origin, Vector3 direction,
                                  float max_dist) const {
  const float step          = 0.1f;
  const float start         = 0.8f;
  const float eye_height    = 2.0f;
  float player_floor_y = get_floor_height(origin.x, origin.z) + eye_height;
  Vector3 last_valid = origin;

  for (float dist = start; dist <= max_dist; dist += step) {
    Vector3 candidate = Vector3Add(origin, Vector3Scale(direction, dist));
    float floor_cam_y = get_floor_height(candidate.x, candidate.z) + eye_height;

    // Hard rule: if the raw ray (before any hugging) is passing through the
    // vertical band of an elevated surface top, the player is looking directly
    // at a pillar top — lock there immediately and show the cone.
    if (floor_cam_y > player_floor_y + 0.5f &&
        candidate.y >= floor_cam_y - 0.5f &&
        candidate.y <= floor_cam_y + 1.0f) {
      Vector3 top = {candidate.x, floor_cam_y, candidate.z};
      if (position_is_acceptable(top))
        return top;
    }

    // Only hug elevated surfaces — when the target floor is HIGHER than the
    // player's start floor (aiming at a ledge/pillar top). Skip hugging when
    // looking down to lower or equal ground so the ball tracks the crosshair.
    if (candidate.y < floor_cam_y && floor_cam_y > player_floor_y)
      candidate.y = floor_cam_y;

    if (is_solid(candidate)) {
      // Pop to surface top only if it's within a reasonable vertical range —
      // prevents jarring jumps when a tall pillar is barely grazed by the ray.
      if (floor_cam_y - candidate.y < 4.0f) {
        Vector3 surface_top = {candidate.x, floor_cam_y, candidate.z};
        if (position_is_acceptable(surface_top))
          last_valid = surface_top;
      }
      break;
    }
    if (position_is_acceptable(candidate))
      last_valid = candidate;
  }

  // Magnetic snap: if last_valid lands within 1.5 units of an elevated surface
  // top, lock to it — makes it easy to intentionally land on ledges.
  float last_floor = get_floor_height(last_valid.x, last_valid.z) + eye_height;
  if (last_floor > player_floor_y + 0.5f &&
      fabsf(last_valid.y - last_floor) < 1.5f) {
    Vector3 snapped = {last_valid.x, last_floor, last_valid.z};
    if (position_is_acceptable(snapped))
      last_valid = snapped;
  }

  return last_valid;
}

// Tap blink variant: scans the full range without stopping at obstacles,
// taking the furthest valid position. Lets the player zip through walls.
Vector3 World::find_blink_target_through(Vector3 origin, Vector3 direction,
                                          float max_dist) const {
  const float step       = 0.1f;
  const float start      = 0.8f;
  const float eye_height = 2.0f;
  Vector3 last_valid = origin;
  for (float dist = start; dist <= max_dist; dist += step) {
    Vector3 candidate = Vector3Add(origin, Vector3Scale(direction, dist));
    float floor_cam_y = get_floor_height(candidate.x, candidate.z) + eye_height;
    if (candidate.y < floor_cam_y)
      candidate.y = floor_cam_y;
    if (position_is_acceptable(candidate))
      last_valid = candidate;
    // no break — keep scanning through obstacles
  }
  return last_valid;
}

bool World::is_ceiling_blocked(Vector3 camera_pos) const {
  // Narrow radius — wall clearance (0.35) keeps us away from wall faces,
  // so this only fires for voxels genuinely above the player's head
  const float radius = 0.15f;
  float head_y = camera_pos.y;
  float xs[] = {camera_pos.x - radius, camera_pos.x + radius};
  float zs[] = {camera_pos.z - radius, camera_pos.z + radius};
  for (float x : xs)
    for (float z : zs)
      if (is_solid({x, head_y, z}))
        return true;
  return false;
}

float World::get_floor_height(float x, float z) const {
  for (int y = 63; y >= 0; --y) {
    if (is_solid({x, (float)y, z}))
      return (float)(y + 1);
  }
  return 0.0f;
}

bool World::position_is_acceptable(Vector3 camera_pos) const {
  const float radius = 0.35f;
  const float eye_height = 2.0f;
  float feet_y = camera_pos.y - eye_height;

  float ys[] = {feet_y + 0.05f, camera_pos.y - eye_height * 0.5f, camera_pos.y};
  float xs[] = {camera_pos.x - radius, camera_pos.x + radius};
  float zs[] = {camera_pos.z - radius, camera_pos.z + radius};

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
  if (dist < 0.01f)
    return true;
  const float step = 0.5f;
  int steps = (int)(dist / step);
  for (int i = 1; i < steps; ++i) {
    float t = (float)i * step / dist;
    Vector3 p = Vector3Add(from, Vector3Scale(delta, t));
    if (is_solid(p))
      return false;
  }
  return true;
}
