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

  if (player_ptr) {
    // Store original position
    Vector2 original_pos = player_ptr->get_position();

    // Try to move the player
    player_ptr->update(dt, true);

    // Check for collisions after attempted movement
    bool colliding = false;
    Vector2 new_pos = player_ptr->get_position();

    for (auto &obj : objects) {
      if (CheckCollisionCircles(new_pos, 40, obj->get_position(), 120)) {
        colliding = true;
        break;
      }
    }

    // If colliding, reset to original position
    if (colliding) {
      player_ptr->set_position(original_pos);
    }

    // Update camera to follow player
    camera.target = player_ptr->get_position();
  }
}

void World::draw() {
  BeginMode2D(camera);

  for (auto &obj : objects) {
    if (is_on_screen(obj->get_position())) {
      obj->draw();
    }
  }
  player_ptr->draw();

  EndMode2D();
}
