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
  chunk_size_ = 64;
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

// ===== Shape Primitive Helpers =====

void World::add_box(Chunk* chunk, int x1, int y1, int z1, int x2, int y2, int z2) {
    for (int y = std::min(y1, y2); y <= std::max(y1, y2); y++) {
        for (int z = std::min(z1, z2); z <= std::max(z1, z2); z++) {
            for (int x = std::min(x1, x2); x <= std::max(x1, x2); x++) {
                chunk->set_voxel(x, y, z);
            }
        }
    }
}

void World::add_hollow_box(Chunk* chunk, int x1, int y1, int z1, int x2, int y2, int z2) {
    int xmin = std::min(x1, x2), xmax = std::max(x1, x2);
    int ymin = std::min(y1, y2), ymax = std::max(y1, y2);
    int zmin = std::min(z1, z2), zmax = std::max(z1, z2);

    for (int y = ymin; y <= ymax; y++) {
        for (int z = zmin; z <= zmax; z++) {
            for (int x = xmin; x <= xmax; x++) {
                // Only fill edges
                bool is_edge = (x == xmin || x == xmax ||
                               z == zmin || z == zmax ||
                               y == ymin || y == ymax);
                if (is_edge) {
                    chunk->set_voxel(x, y, z);
                }
            }
        }
    }
}

void World::add_floor(Chunk* chunk, int x1, int z1, int x2, int z2, int y) {
    for (int z = std::min(z1, z2); z <= std::max(z1, z2); z++) {
        for (int x = std::min(x1, x2); x <= std::max(x1, x2); x++) {
            chunk->set_voxel(x, y, z);
        }
    }
}

void World::add_pillar(Chunk* chunk, int x, int z, int y1, int y2) {
    for (int y = std::min(y1, y2); y <= std::max(y1, y2); y++) {
        chunk->set_voxel(x, y, z);
    }
}

void World::add_wall_x(Chunk* chunk, int z, int y1, int y2, int x1, int x2) {
    for (int y = std::min(y1, y2); y <= std::max(y1, y2); y++) {
        for (int x = std::min(x1, x2); x <= std::max(x1, x2); x++) {
            chunk->set_voxel(x, y, z);
        }
    }
}

void World::add_wall_z(Chunk* chunk, int x, int y1, int y2, int z1, int z2) {
    for (int y = std::min(y1, y2); y <= std::max(y1, y2); y++) {
        for (int z = std::min(z1, z2); z <= std::max(z1, z2); z++) {
            chunk->set_voxel(x, y, z);
        }
    }
}

bool World::load_chunk_data(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << "\n";
        return false;
    }

    Chunk* current_chunk = nullptr;
    int line_num = 0;
    std::string line;

    while (std::getline(file, line)) {
        line_num++;

        // Trim leading whitespace
        size_t start = line.find_first_not_of(" \t");
        if (start == std::string::npos) continue; // Empty line
        line = line.substr(start);

        // Skip comments and empty lines
        if (line.empty() || line[0] == '#') continue;

        std::istringstream ss(line);
        std::string command;
        ss >> command;

        // Parse chunk command
        if (command == "chunk") {
            int cx, cy;
            if (!(ss >> cx >> cy)) {
                std::cerr << "Line " << line_num << ": Invalid chunk command\n";
                continue;
            }
            current_chunk = get_or_create_chunk(cx, cy);
        }
        // Parse shape primitives
        else if (command == "box") {
            if (!current_chunk) {
                std::cerr << "Line " << line_num << ": No active chunk\n";
                continue;
            }
            int x1, y1, z1, x2, y2, z2;
            if (!(ss >> x1 >> y1 >> z1 >> x2 >> y2 >> z2)) {
                std::cerr << "Line " << line_num << ": Invalid box command\n";
                continue;
            }
            add_box(current_chunk, x1, y1, z1, x2, y2, z2);
        }
        else if (command == "hollow_box") {
            if (!current_chunk) {
                std::cerr << "Line " << line_num << ": No active chunk\n";
                continue;
            }
            int x1, y1, z1, x2, y2, z2;
            if (!(ss >> x1 >> y1 >> z1 >> x2 >> y2 >> z2)) {
                std::cerr << "Line " << line_num << ": Invalid hollow_box command\n";
                continue;
            }
            add_hollow_box(current_chunk, x1, y1, z1, x2, y2, z2);
        }
        else if (command == "floor") {
            if (!current_chunk) {
                std::cerr << "Line " << line_num << ": No active chunk\n";
                continue;
            }
            int x1, z1, x2, z2, y = 0;
            if (!(ss >> x1 >> z1 >> x2 >> z2)) {
                std::cerr << "Line " << line_num << ": Invalid floor command\n";
                continue;
            }
            ss >> y; // Optional y parameter
            add_floor(current_chunk, x1, z1, x2, z2, y);
        }
        else if (command == "pillar") {
            if (!current_chunk) {
                std::cerr << "Line " << line_num << ": No active chunk\n";
                continue;
            }
            int x, z, y1, y2;
            if (!(ss >> x >> z >> y1 >> y2)) {
                std::cerr << "Line " << line_num << ": Invalid pillar command\n";
                continue;
            }
            add_pillar(current_chunk, x, z, y1, y2);
        }
        else if (command == "wall_x") {
            if (!current_chunk) {
                std::cerr << "Line " << line_num << ": No active chunk\n";
                continue;
            }
            int z, y1, y2, x1, x2;
            if (!(ss >> z >> y1 >> y2 >> x1 >> x2)) {
                std::cerr << "Line " << line_num << ": Invalid wall_x command\n";
                continue;
            }
            add_wall_x(current_chunk, z, y1, y2, x1, x2);
        }
        else if (command == "wall_z") {
            if (!current_chunk) {
                std::cerr << "Line " << line_num << ": No active chunk\n";
                continue;
            }
            int x, y1, y2, z1, z2;
            if (!(ss >> x >> y1 >> y2 >> z1 >> z2)) {
                std::cerr << "Line " << line_num << ": Invalid wall_z command\n";
                continue;
            }
            add_wall_z(current_chunk, x, y1, y2, z1, z2);
        }
        else if (command == "voxel") {
            if (!current_chunk) {
                std::cerr << "Line " << line_num << ": No active chunk\n";
                continue;
            }
            int x, y, z;
            if (!(ss >> x >> y >> z)) {
                std::cerr << "Line " << line_num << ": Invalid voxel command\n";
                continue;
            }
            current_chunk->set_voxel(x, y, z);
        }
        else {
            std::cerr << "Line " << line_num << ": Unknown command '" << command << "'\n";
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
