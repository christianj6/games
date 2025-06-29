#include "world.h"
#include "enemy.h"

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

bool World::update(float dt) {
  Vector2 player_pos = player_ptr->get_position();
  bool gameover;
  for (auto &obj : objects) {
    // Check if object is an enemy using dynamic_cast
    if (auto enemy = dynamic_cast<Enemy *>(obj.get())) {
      enemy->update_goap(dt, player_pos, get_obstacles());
      gameover = enemy->is_touching_player(player_pos);
    }
    obj->update(dt);
  }
  if (gameover) {
    return false;
  }

  if (player_ptr) {
    // Store original position
    Vector2 original_pos = player_ptr->get_position();

    // Try to move the player
    player_ptr->update(dt, true);
    Vector2 new_pos = player_ptr->get_position();

    for (auto &obj : objects) {
      if (auto enemy = dynamic_cast<Enemy *>(obj.get())) {
        break;
      }
      if (CheckCollisionCircles(new_pos, 40, obj->get_position(), 120)) {
        Vector2 diff = {new_pos.x - obj->get_position().x,
                        new_pos.y - obj->get_position().y};

        // Normalize the direction
        float length = sqrtf(diff.x * diff.x + diff.y * diff.y);
        if (length > 0) {
          diff.x /= length;
          diff.y /= length;
        }

        // lightly 'bounce' the player off the obstacle
        const float bounce = 30.0f;
        player_ptr->set_position({original_pos.x + diff.x * bounce,
                                  original_pos.y + diff.y * bounce});
      }
    }

    // Update camera to follow player
    camera.target = player_ptr->get_position();
  }
  return true;
}

std::vector<GameObject *> World::get_obstacles() const {
  std::vector<GameObject *> obstacles;
  for (const auto &obj : objects) {
    if (!dynamic_cast<Enemy *>(obj.get())) {
      obstacles.push_back(obj.get());
    }
  }
  return obstacles;
}

void World::draw() {
  BeginMode2D(camera);

  for (auto &obj : objects) {
    if (is_on_screen(obj->get_position()) || dynamic_cast<Enemy *>(obj.get())) {
      obj->draw();
    }
  }
  player_ptr->draw();

  EndMode2D();

  // Draw screen-space elements after EndMode2D
  for (auto &obj : objects) {
    if (auto enemy = dynamic_cast<Enemy *>(obj.get())) {
      enemy->draw_indicator(player_ptr->get_position());
    }
  }
}
