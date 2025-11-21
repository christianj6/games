#include "world.h"
#include "raylib.h"
#include <memory>
#include <fstream>
#include <iostream>
#include <sstream>

World::World() {
  chunk_size_ = 8;

  load_chunk_data("resources/maps/home.txt");
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
  // TODO: decide which chunks are loaded
}

void World::draw() {
  for (auto &c : chunks_) {
    if (c->loaded) {
      c->draw();
    }
  }
}

bool World::position_is_acceptable(Vector3 position) {
  // TODO
  return true;
}
