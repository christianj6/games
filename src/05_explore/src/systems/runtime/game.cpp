#include "game.h"
#include "raylib.h"
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
  RandomNumberGenerator<float> angle(0.0f, 6.2831853f);
  RandomNumberGenerator<float> dist(120.0f, 300.0f);
  const float world_min = 8.0f;
  const float world_max = (float)(64 * 16) - 8.0f;
  for (int i = 0; i < blackboard.quest.required; ++i) {
    float a = angle();
    float d = dist();
    float x = cosf(a) * d;
    float z = sinf(a) * d;
    if (x < world_min) x = world_min;
    if (x > world_max) x = world_max;
    if (z < world_min) z = world_min;
    if (z > world_max) z = world_max;
    float y = world.get_floor_height(x, z) + 0.35f;
    items_.push_back(std::make_unique<Item>(Vector3{x, y, z}));
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
