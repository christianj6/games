#include "world.h"
#include "raylib.h"
#include <memory>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cmath>
#include <chrono>
#include <thread>
#include <cctype>
#include "utils/random.h"
#include "rlights.h"
#include "raymath.h"

void World::make_random_pillars(Chunk* chunk) {
  int pillar_probability = 2;
  RandomNumberGenerator<int> is_pillar(0, 100);
  RandomNumberGenerator<int> random_height(1, 20);
  for (int x = 0; x < chunk_size_; ++x) {
    for (int y = 0; y < chunk_size_; ++y) {
      if (is_pillar() < pillar_probability) {
        add_pillar(chunk, x, y, random_height() + 1);
      }
      else {
        add_pillar(chunk, x, y, 1);
      }
    }
  }
}

World::World() : should_exit_(false) {
  chunk_size_ = 64;
  world_size_chunks_ = 8;
  render_distance_ = 3;  // Load chunks within 3 chunks of player

  // Start the chunk loading thread
  loading_thread_ = std::thread(&World::chunk_loading_worker, this);
}

void World::build_chunks() {
  load_chunk_data("maps/home.txt");
  for (int i = 0; i < world_size_chunks_; ++i) {
    for (int j = 0; j < world_size_chunks_; ++j) {
      // we hard-coded chunk 0,0
      if (i == 0 && j == 0) continue;
      Chunk* current_chunk = get_or_create_chunk(i, j);
      // fill the chunk with random pillars 
      make_random_pillars(current_chunk);
    }
  }
  // TODO: better lighting
  CreateLight(LIGHT_POINT, Vector3{50.0f, 10.0f, 0}, Vector3Zero(),
              DARKPURPLE, renderer_->get_shader());
}

World::~World() {
  // Signal the thread to exit
  should_exit_ = true;

  // Wait for the thread to finish
  if (loading_thread_.joinable()) {
    loading_thread_.join();
  }
}

void World::set_renderer(Renderer* renderer) {
  renderer_ = renderer;
}

Chunk* World::get_or_create_chunk(int cx, int cy) {
    for (auto& c : chunks_) {
        if (c->get_position().x == cx && c->get_position().y == cy)
            return c.get();
    }
    // Not found → create new
    auto chunk = std::make_unique<Chunk>(Vector2{float(cx), float(cy)}, chunk_size_, renderer_);
    Chunk* ptr = chunk.get();
    chunks_.push_back(std::move(chunk));
    return ptr;
}

void World::add_pillar(Chunk* chunk, int x, int z, int height) {
  for (int y = 0; y < height; ++y) {
    chunk->set_voxel(x, y, z);
  }
}

bool is_all_digits(const std::string& s) {
    return !s.empty() &&
        std::all_of(s.begin(), s.end(),
            [](unsigned char c){ return std::isdigit(c); });
}

std::vector<std::string> split_whitespace(const std::string& s) {
    std::istringstream iss(s);
    std::vector<std::string> tokens;
    std::string token;

    while (iss >> token) {   // operator>> automatically skips whitespace
        tokens.push_back(token);
    }

    return tokens;
}

bool World::load_chunk_data(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << "\n";
        return false;
    }

    Chunk* current_chunk = nullptr;
    int current_chunk_x = 0;
    int current_chunk_y = 0;

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

        if (command == "chunk") {
            int cx, cy;
            if (!(ss >> cx >> cy)) {
                std::cerr << "Line " << line_num << ": Invalid chunk command\n";
                continue;
            }
            current_chunk = get_or_create_chunk(cx, cy);
            current_chunk_x = 0;
            current_chunk_y = 0;
        }
        else if (is_all_digits(command)) {
          std::vector<std::string> tokens = split_whitespace(line);
          for (auto& token : tokens) {
            int height = std::stoi(token);
            // always add a "pillar" (if value is zero, add a single block as a floor)
            add_pillar(current_chunk, current_chunk_x, current_chunk_y, height+1);
            current_chunk_x++;
          }
          current_chunk_x = 0;
          current_chunk_y++;
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
