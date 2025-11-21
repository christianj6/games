#include "world.h"
#include "raylib.h"
#include <memory>

World::World() {
  // TODO: construct the chunks
  // chunks.push_back(std::make_unique<Chunk>);
}
void World::update(float dt, Vector3 current_player_position) {
  // TODO: decide which chunks are loaded
}

void World::draw() {
  // TODO: draw the chunks
  DrawCubeWires({1.f, 3.f, 1.f}, 5, 5, 5, BLUE); // TODO: remove
  for (auto &c : chunks) {
    if (c->loaded) {
      c->draw();
    }
  }
}

bool World::position_is_acceptable(Vector3 position) {
  // TODO
  return true;
}
