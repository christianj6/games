#include "world.h"
#include "raylib.h"
#include <memory>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cmath>
#include <chrono>
#include <thread>

World::World() : should_exit_(false) {
  chunk_size_ = 8;
  render_distance_ = 3;  // Load chunks within 3 chunks of player

  load_chunk_data("resources/maps/home.txt");

  // Start the chunk loading thread
  loading_thread_ = std::thread(&World::chunk_loading_worker, this);
}

World::~World() {
  // Signal the thread to exit
  should_exit_ = true;

  // Wait for the thread to finish
  if (loading_thread_.joinable()) {
    loading_thread_.join();
  }
}

Chunk* World::get_or_create_chunk(int cx, int cy) {
    for (auto& c : chunks_) {
        if (c->get_position().x == cx && c->get_position().y == cy)
            return c.get();
    }
    // Not found → create new
    auto chunk = std::make_unique<Chunk>(Vector2{float(cx), float(cy)}, chunk_size_);
    Chunk* ptr = chunk.get();
    chunks_.push_back(std::move(chunk));
    return ptr;
}

bool World::load_chunk_data(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << "\n";
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Skip comments or empty lines
        if (line.empty() || line[0] == '#') continue;

        std::istringstream ss(line);
        int chunkX, chunkY, x, y, z;
        if (!(ss >> chunkX >> chunkY >> x >> y >> z)) {
            std::cerr << "Invalid line in file: " << line << "\n";
            continue;
        }

        Chunk* chunk = get_or_create_chunk(chunkX, chunkY);
        chunk->set_voxel(x, y, z);
    }

    // // Build meshes for all chunks
    // for (auto& c : chunks_) {
    //     c->load();
    // }

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
    bool has_work = false;

    // Check if there's work to do
    {
      std::lock_guard<std::mutex> lock(load_queue_mutex_);
      if (!load_queue_.empty()) {
        request = load_queue_.front();
        load_queue_.pop();
        has_work = true;
      }
    }

    if (has_work) {
      // Generate mesh (CPU-intensive, thread-safe)
      request.chunk_ptr->generate_mesh();
    } else {
      // Sleep briefly to avoid busy-waiting
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }
}

void World::update_chunk_loading(Vector3 player_position) {
  // Convert player position to chunk coordinates
  int player_chunk_x = static_cast<int>(std::floor(player_position.x / chunk_size_));
  int player_chunk_z = static_cast<int>(std::floor(player_position.z / chunk_size_));

  std::lock_guard<std::mutex> lock(chunks_mutex_);

  // Upload chunks that are ready (must be done on main thread)
  for (auto& chunk : chunks_) {
    if (chunk->state == ChunkState::READY_TO_UPLOAD) {
      chunk->upload_mesh();
    }
  }

  // Unload chunks that are too far away
  for (auto& chunk : chunks_) {
    int chunk_x = static_cast<int>(chunk->get_position().x);
    int chunk_y = static_cast<int>(chunk->get_position().y);

    int dx = chunk_x - player_chunk_x;
    int dy = chunk_y - player_chunk_z;
    float distance = std::sqrt(dx * dx + dy * dy);

    if (distance > render_distance_ && chunk->state == ChunkState::LOADED) {
      chunk->unload();
    }
  }

  // Queue chunks for loading that are within range
  for (auto& chunk : chunks_) {
    int chunk_x = static_cast<int>(chunk->get_position().x);
    int chunk_y = static_cast<int>(chunk->get_position().y);

    int dx = chunk_x - player_chunk_x;
    int dy = chunk_y - player_chunk_z;
    float distance = std::sqrt(dx * dx + dy * dy);

    if (distance <= render_distance_ && chunk->state == ChunkState::UNLOADED) {
      // Mark as generating immediately to prevent duplicate queueing
      chunk->state = ChunkState::GENERATING;

      // Add to loading queue for background processing
      std::lock_guard<std::mutex> queue_lock(load_queue_mutex_);
      load_queue_.push({chunk_x, chunk_y, chunk.get()});
    }
  }
}

bool World::position_is_acceptable(Vector3 position) {
  // TODO
  return true;
}
