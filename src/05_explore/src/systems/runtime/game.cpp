#include "game.h"
#include "raylib.h"
#include "raymath.h"
#include "systems/runtime/audio.h"
#include "utils/random.h"
#include <cmath>
#include <memory>

namespace {
constexpr float kRegenDelay = 5.0f;
constexpr float kRegenRate = 3.0f;
constexpr float kDamageFlashDecay = 3.0f;
constexpr float kReviveSearchRadius = 50.0f;
constexpr int kShardsRequired = 5;
constexpr float kShardMinDist = 200.0f;
constexpr float kShardMaxDist = 460.0f;
constexpr float kShardSpawnClearance = 180.0f;
constexpr float kShardSpread = 100.0f;
constexpr float kMaxWalkableHeight = 12.0f;
constexpr int kShardPlaceAttempts = 60;
constexpr int kGuardPlaceAttempts = 24;
constexpr float kGuardMinOffset = 4.0f;
constexpr float kGuardOffsetStep = 2.0f;
constexpr float kGuardAngleStep = 0.9f;
constexpr float kItemHoverHeight = 0.35f;
constexpr float kWorldExtent = 64.0f * 16.0f; // chunk count × chunk size
// The shard and guard world-bounds clamps use different margins on purpose:
// shards keep a wider buffer from the world edge than guard posts.
constexpr float kShardBoundsMargin = 16.0f;
constexpr float kGuardBoundsMargin = 8.0f;
constexpr int kGuardsPerShard = 2;
} // namespace

Game::Game() {
  Audio::get().init(); // device must be ready before enemies load their hum

  blackboard.world = &world;

  // TODO: remove sequential coupling
  world.set_renderer(&renderer);
  world.build_chunks();

  friend_ = std::make_unique<Friend>(&renderer);
  blackboard.quest.required = kShardsRequired;
  spawn_quest_items();
}

Game::~Game() { Audio::get().stop_ambient(); }

GameInfo Game::tick() {
  GameInfo info{};
  if (current_state_ == GameState::RUNNING)
    update();

  if (current_state_ == GameState::RUNNING &&
      (IsKeyPressed(KEY_ESCAPE) ||
       IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT))) {
    toggle_pause();
  } else if (current_state_ == GameState::PAUSED) {
    // Pause menu
    // Click also resumes: on web the click gesture lets the main loop's
    // pending pointer-lock request engage immediately after ESC.
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER) ||
        IsMouseButtonPressed(MOUSE_BUTTON_LEFT) ||
        IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) {
      toggle_pause(); // resume
    } else if (IsKeyPressed(KEY_R) ||
               IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_UP)) {
      info.restart = true;
    } else if (IsKeyPressed(KEY_Q) ||
               IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_LEFT)) {
      info.quit_to_menu = true;
    }
  }

  draw();

  return info;
}

void Game::update() {
  float dt = GetFrameTime();

  blackboard.current_player_position = player.update(dt, blackboard).position;
  float scaled_dt = dt * blackboard.time_scale;
  world.update(scaled_dt, blackboard.current_player_position);
  Audio::get().update(); // ambient wind stream

  update_listener();

  blackboard.friend_nearby = false;
  blackboard.takedown_available = false;
  blackboard.attack_available = false;
  for (auto &a : actors) {
    a->update(scaled_dt, blackboard);
  }
  pair_revive_targets();
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

  update_health(dt);
}

// Listener orientation for pseudo-spatial audio panning.
void Game::update_listener() {
  Camera3D cam = player.get_camera();
  Vector3 cam_fwd = Vector3Normalize(Vector3Subtract(cam.target, cam.position));
  blackboard.listener_right =
      Vector3Normalize(Vector3CrossProduct(cam_fwd, {0.0f, 1.0f, 0.0f}));
}

// Guard revival: nearest standing guard walks to a downed ally and lifts
// them after a moment. Takes that guard off patrol — a stealth reward.
void Game::pair_revive_targets() {
  // One typed non-owning view so the pairing below runs cast-free.
  std::vector<Enemy *> enemies;
  enemies.reserve(actors.size());
  for (auto &a : actors) {
    if (auto *e = dynamic_cast<Enemy *>(a.get()))
      enemies.push_back(e);
  }

  for (auto *e : enemies) {
    if (!e->is_incapacitated())
      e->set_revive_target(nullptr);
  }
  for (auto *down : enemies) {
    if (!down->is_incapacitated())
      continue;
    Enemy *best = nullptr;
    float best_d = kReviveSearchRadius;
    for (auto *e : enemies) {
      if (e == down || e->is_incapacitated())
        continue;
      float d = Vector3Length(
          Vector3Subtract(e->get_position(), down->get_position()));
      if (d < best_d) {
        best_d = d;
        best = e;
      }
    }
    if (best != nullptr)
      best->set_revive_target(down);
  }
}

// Damage feedback decay and LOSE
void Game::update_health(float dt) {
  if (blackboard.damage_flash > 0.0f)
    blackboard.damage_flash =
        std::fmax(0.0f, blackboard.damage_flash - dt * kDamageFlashDecay);

  // Slow health regeneration after 5s without taking damage.
  if (blackboard.player_health < last_health_)
    time_since_damage_ = 0.0f;
  else
    time_since_damage_ += dt;
  last_health_ = blackboard.player_health;
  if (time_since_damage_ > kRegenDelay && blackboard.player_health > 0.0f)
    blackboard.player_health =
        std::fmin(kMaxPlayerHealth, blackboard.player_health + kRegenRate * dt);

  if (blackboard.player_health <= 0.0f) {
    blackboard.player_health = 0.0f;
    Audio::get().play(Sfx::Lose, 0.8f);
    current_state_ = GameState::LOSE;
  }
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

  if (current_state_ == GameState::PAUSED) {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), {0, 0, 0, 170});
    const char *title = "PAUSED";
    int tw = MeasureText(title, 50);
    DrawText(title, GetScreenWidth() / 2 - tw / 2, GetScreenHeight() / 2 - 120,
             50, WHITE);
    const char *options[] = {"ESC / CLICK / A  -  Resume", "R / Y  -  Restart",
                             "Q / X  -  Quit to Menu"};
    for (int i = 0; i < 3; ++i) {
      int w = MeasureText(options[i], 24);
      DrawText(options[i], GetScreenWidth() / 2 - w / 2,
               GetScreenHeight() / 2 - 40 + i * 38, 24, LIGHTGRAY);
    }
  }
  if (current_state_ == GameState::WIN || current_state_ == GameState::LOSE) {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), {0, 0, 0, 160});
    const char *msg =
        current_state_ == GameState::WIN ? "YOU WIN!" : "YOU LOSE";
    int mw = MeasureText(msg, 60);
    DrawText(msg, GetScreenWidth() / 2 - mw / 2, GetScreenHeight() / 2 - 60, 60,
             current_state_ == GameState::WIN ? GOLD : RED);
    const char *hint = "Press ENTER for menu";
    int hw = MeasureText(hint, 24);
    DrawText(hint, GetScreenWidth() / 2 - hw / 2, GetScreenHeight() / 2 + 20,
             24, WHITE);
  }

  hud.draw(blackboard);
}

void Game::toggle_pause() {
  if (current_state_ != GameState::PAUSED) {
    previous_state_ = current_state_;
    current_state_ = GameState::PAUSED;
  } else if (current_state_ == GameState::PAUSED) {
    current_state_ = previous_state_;
  }
}

void Game::spawn_quest_items() {
  const Vector3 player_spawn = {20.0f, 0.0f, 20.0f};
  std::vector<Vector3> placed;

  for (int i = 0; i < blackboard.quest.required; ++i) {
    Vector3 pos = sample_shard_position(player_spawn, placed);
    placed.push_back(pos);

    float y = world.get_floor_height(pos.x, pos.z) + kItemHoverHeight;
    items_.push_back(std::make_unique<Item>(Vector3{pos.x, y, pos.z}));
    spawn_shard_guards(pos);
  }
}

Vector3 Game::sample_shard_position(const Vector3 &player_spawn,
                                    const std::vector<Vector3> &placed) {
  const float world_min = kShardBoundsMargin;
  const float world_max = kWorldExtent - kShardBoundsMargin;
  RandomNumberGenerator<float> angle(0.0f, 2.0f * PI);
  RandomNumberGenerator<float> dist(kShardMinDist, kShardMaxDist);
  Vector3 pos{};
  for (int attempt = 0; attempt < kShardPlaceAttempts; ++attempt) {
    float x = cosf(angle()) * dist();
    float z = sinf(angle()) * dist();
    if (x < world_min)
      x = world_min;
    if (x > world_max)
      x = world_max;
    if (z < world_min)
      z = world_min;
    if (z > world_max)
      z = world_max;
    pos = {x, 0.0f, z};

    // Keep shards away from the player spawn and spread apart from each
    // other so exploration covers the whole world, not one quadrant.
    Vector3 to_spawn = Vector3Subtract(pos, player_spawn);
    if (Vector3Length(to_spawn) < kShardSpawnClearance)
      continue;
    // Reachability: shards on spire tops (>= ~25 u) are beyond double-jump
    // + blink range — never place them there.
    if (world.get_floor_height(pos.x, pos.z) > kMaxWalkableHeight)
      continue;
    bool clear = true;
    for (const auto &p : placed) {
      if (Vector3Length(Vector3Subtract(pos, p)) < kShardSpread) {
        clear = false;
        break;
      }
    }
    if (clear)
      break;
  }
  return pos;
}

// Multiple guards per shard, posted on opposite sides. Posts must be on
// walkable ground — never on top of a pillar.
void Game::spawn_shard_guards(const Vector3 &shard_pos) {
  RandomNumberGenerator<float> guard_angle(0.0f, 2.0f * PI);
  float ga = guard_angle();
  for (int g = 0; g < kGuardsPerShard; ++g) {
    float base_a = ga + g * PI; // opposite posts
    Vector3 post = shard_pos;
    bool found = false;
    for (int attempt = 0; attempt < kGuardPlaceAttempts && !found; ++attempt) {
      float a = base_a + attempt * kGuardAngleStep;
      float off = kGuardMinOffset + (attempt % 5) * kGuardOffsetStep;
      float gx = shard_pos.x + cosf(a) * off;
      float gz = shard_pos.z + sinf(a) * off;
      if (gx < kGuardBoundsMargin)
        gx = kGuardBoundsMargin;
      if (gx > kWorldExtent - kGuardBoundsMargin)
        gx = kWorldExtent - kGuardBoundsMargin;
      if (gz < kGuardBoundsMargin)
        gz = kGuardBoundsMargin;
      if (gz > kWorldExtent - kGuardBoundsMargin)
        gz = kWorldExtent - kGuardBoundsMargin;
      float h = world.get_floor_height(gx, gz);
      // Prefer plain ground; rubble pillar tops (<= 5) as last resort.
      if ((attempt < 12 && h <= 2.5f) || (attempt >= 12 && h <= 5.0f)) {
        post = Vector3{gx, 0.0f, gz};
        found = true;
      }
    }
    actors.push_back(std::make_unique<Enemy>(&renderer, post));
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
      blackboard.quest.collected >= blackboard.quest.required) {
    blackboard.quest.state = QuestState::TURN_IN;
    Audio::get().play(Sfx::TurnIn, 0.7f);
  }

  if (blackboard.quest.state == QuestState::TURN_IN &&
      blackboard.friend_nearby &&
      (IsKeyPressed(KEY_E) ||
       IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_LEFT))) {
    blackboard.quest.state = QuestState::COMPLETE;
    Audio::get().play(Sfx::Win, 0.8f);
    current_state_ = GameState::WIN;
  }
}
