#include "world.h"

bool World::is_on_screen(const Vector2 &position) const {
  float margin = 100.0f; // Extra buffer zone around screen
  Vector2 screen_pos = GetWorldToScreen2D(position, camera);
  Rectangle screen_bounds = {-margin, -margin, GetScreenWidth() + (margin * 2),
                             GetScreenHeight() + (margin * 2)};
  return CheckCollisionPointRec(screen_pos, screen_bounds);
}

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
  // DEBUG: Count visible objects
  int visible_count = 0;

  BeginMode2D(camera);

  for (auto &obj : objects) {
    if (is_on_screen(obj->get_position())) {
      obj->draw();
      visible_count++; // DEBUG: Increment counter for each visible object
    }
  }
  player_ptr->draw();

  EndMode2D();

  // DEBUG: Draw object count (will be removed later)
  DrawText(TextFormat("Visible Objects: %d/%zu", visible_count, objects.size()),
           10, 10, 20, WHITE);
}
