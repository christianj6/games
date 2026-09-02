#include "game.h"
#include "raylib.h"
#include "raymath.h"
#include "utils/random.h"
#include <cmath>
#include <memory>


Game::Game() {
  blackboard.world = &world;

  // TODO: remove sequential coupling
  world.set_renderer(&renderer);
  world.build_chunks();

  friend_ = std::make_unique<Friend>(&renderer);
  blackboard.quest.required = 5;
  spawn_quest_items();
}


GameInfo Game::tick(bool debug) {
  if (current_state == GameState::RUNNING) {
    update();
  }
  if (current_state == GameState::RUNNING &&
      (IsKeyPressed(KEY_ESCAPE) ||
       IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT))) {
    toggle_pause();
  }

  draw();

  return {};
}

GameInfo Game::update() {
  float dt = GetFrameTime();

  blackboard.current_player_position = player.update(dt, blackboard).position;
  float scaled_dt = dt * blackboard.time_scale;
  world.update(scaled_dt, blackboard.current_player_position);
  renderer.update(player.get_camera());

  blackboard.friend_nearby = false;
  for (auto &a : actors) {
    a->update(scaled_dt, blackboard);
  }
  friend_->update(scaled_dt, blackboard);
  for (auto &item : items_) {
    item->update(scaled_dt, blackboard);
  }
  update_quest();

  // Remove executed guards
  for (auto it = actors.begin(); it != actors.end();) {
    if ((*it)->is_dead())
      it = actors.erase(it);
    else
      ++it;
  }

  // Damage feedback decay and LOSE
  if (blackboard.damage_flash > 0.0f)
    blackboard.damage_flash = std::fmax(0.0f, blackboard.damage_flash - dt * 3.0f);
  if (blackboard.player_health <= 0.0f) {
    blackboard.player_health = 0.0f;
    current_state = GameState::LOSE;
  }

  return {};
}

void Game::draw() {
  BeginMode3D(player.get_camera());

  player.draw();
  world.draw();
  friend_->draw();
  for (auto &item : items_) {
    item->draw();
  }
  for (auto &a : actors) {
    a->draw();
  }

  EndMode3D();

  player.draw_hud(player.get_camera());

  if (current_state == GameState::PAUSED) {
    DrawText("PAUSED", 20, 20, 20, RED);
  }
  if (current_state == GameState::WIN || current_state == GameState::LOSE) {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), {0, 0, 0, 160});
    const char *msg = current_state == GameState::WIN ? "YOU WIN!" : "YOU LOSE";
    int mw = MeasureText(msg, 60);
    DrawText(msg, GetScreenWidth() / 2 - mw / 2, GetScreenHeight() / 2 - 60, 60,
             current_state == GameState::WIN ? GOLD : RED);
    const char *hint = "Press ENTER for menu";
    int hw = MeasureText(hint, 24);
    DrawText(hint, GetScreenWidth() / 2 - hw / 2, GetScreenHeight() / 2 + 20, 24,
             WHITE);
  }


  hud.draw(blackboard);
}

void Game::toggle_pause() {
  if (current_state != GameState::PAUSED) {
    previous_state = current_state;
    current_state = GameState::PAUSED;
  } else if (current_state == GameState::PAUSED) {
    current_state = previous_state;
  }
}

void Game::spawn_quest_items() {
  const Vector3 player_spawn = {20.0f, 0.0f, 20.0f};
  const float world_min = 16.0f;
  const float world_max = (float)(64 * 16) - 16.0f;
  RandomNumberGenerator<float> angle(0.0f, 6.2831853f);
  RandomNumberGenerator<float> dist(200.0f, 460.0f);
  std::vector<Vector3> placed;

  for (int i = 0; i < blackboard.quest.required; ++i) {
    Vector3 pos{};
    for (int attempt = 0; attempt < 60; ++attempt) {
      float x = cosf(angle()) * dist();
      float z = sinf(angle()) * dist();
      if (x < world_min) x = world_min;
      if (x > world_max) x = world_max;
      if (z < world_min) z = world_min;
      if (z > world_max) z = world_max;
      pos = {x, 0.0f, z};

      // Keep shards away from the player spawn and spread apart from each
      // other so exploration covers the whole world, not one quadrant.
      Vector3 to_spawn = Vector3Subtract(pos, player_spawn);
      if (Vector3Length(to_spawn) < 180.0f)
        continue;
      bool clear = true;
      for (const auto &p : placed) {
        if (Vector3Length(Vector3Subtract(pos, p)) < 100.0f) {
          clear = false;
          break;
        }
      }
      if (clear)
        break;
    }
    placed.push_back(pos);

    float y = world.get_floor_height(pos.x, pos.z) + 0.35f;
    items_.push_back(std::make_unique<Item>(Vector3{pos.x, y, pos.z}));
    // Multiple guards per shard, posted on opposite sides, patrolling it.
    RandomNumberGenerator<float> guard_angle(0.0f, 6.2831853f);
    float ga = guard_angle();
    constexpr int kGuardsPerShard = 2;
    for (int g = 0; g < kGuardsPerShard; ++g) {
      float a = ga + g * 3.14159265f; // opposite posts
      actors.push_back(std::make_unique<Enemy>(
          &renderer,
          Vector3{pos.x + cosf(a) * 5.0f, 0.0f, pos.z + sinf(a) * 5.0f}));
    }
  }
}

void Game::update_quest() {
  for (auto it = items_.begin(); it != items_.end();) {
    if ((*it)->is_collected()) {
      if (blackboard.quest.state == QuestState::COLLECTING)
        blackboard.quest.collected++;
      it = items_.erase(it);
    } else {
      ++it;
    }
  }
  if (blackboard.quest.state == QuestState::COLLECTING &&
      blackboard.quest.collected >= blackboard.quest.required)
    blackboard.quest.state = QuestState::TURN_IN;

  if (blackboard.quest.state == QuestState::TURN_IN &&
      blackboard.friend_nearby &&
      (IsKeyPressed(KEY_E) ||
       IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_LEFT))) {
    blackboard.quest.state = QuestState::COMPLETE;
    current_state = GameState::WIN;
  }
}
