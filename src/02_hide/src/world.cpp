#include "world.h"

void World::add_object(std::unique_ptr<GameObject> object) {
  // std::move is required because unique_ptr can't be copied, only moved
  objects.push_back(std::move(object));
}

void World::update(float dt) {
  for (auto &obj : objects) {
    obj->update(dt);
  }
  player_ptr->update(dt);

  // Update camera to follow player
  if (player_ptr) {
    camera.target = player_ptr->get_position();
  }
}

void World::draw() {
  BeginMode2D(camera);

  for (auto &obj : objects) {
    obj->draw();
  }
  player_ptr->draw();

  EndMode2D();
}
