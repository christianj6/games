#include "game.h"
#include "actors/enemy.h"
#include "movement/input.h"
#include "raylib.h"
#include "utils/resource_dir.h"

Game::Game(bool debug_mode_enabled)
    : hud(debug_mode_enabled), world(), map(),
      global_player_visibility_flag(false) {
  SearchAndSetResourceDir("resources");
  // try to place 20 enemies
  const int n_enemies = 7;
  pather = new micropather::MicroPather(&map, 250);
  pather->Reset();
  // because world is static we can set the world data once
  map.set_vector_space_data(world.get_voxel_space_data());
  for (int i = 0; i < n_enemies; i++) {
    Enemy enemy(pather);
    Vector3 pos = enemy.get_position();
    auto voxel_space = world.get_voxel_space_data();

    // Check 3x3 area around spawn position for voxels
    bool clear_area = true;
    for (int dx = -1; dx <= 1 && clear_area; dx++) {
      for (int dz = -1; dz <= 1 && clear_area; dz++) {
        int check_x = static_cast<int>(pos.x) + dx;
        int check_z = static_cast<int>(pos.z) + dz;
        if (voxel_space[1](check_x, check_z)) {
          clear_area = false;
        }
      }
    }

    if (clear_area) {
      // if no collision, place the enemy
      // otherwise give up; its simpler than a retry mechanism and isnt a big
      // deal for this kind of game
      enemies.push_back(std::move(enemy)); // need to move bc of how BT works
    }
  }
}

int Game::update() {
  if (IsGamepadAvailable(0)) {
    player.set_movement_controller(std::make_unique<PlayerMovementController>(
        std::make_unique<ControllerInputProvider>()));
  }
  // noticing that i want to pass different signals between the actors
  // and resort to hacks or unmanaged data passing to do this; in a next
  // iteration we likely need to extend the gameplay layer to handle this
  float dt = GetFrameTime();
  PlayerAction player_actions = player.update(dt, world.get_voxel_space_data());
  world.update(dt, player.get_camera());
  Vector3 current_player_position = player.get_position();
  global_player_visibility_flag =
      false; // reset to false unless any enemy sees player
  for (auto &enemy : enemies) {
    EnemySignals enemy_signals;
    if (!player_actions.blink) {
      enemy_signals = enemy.update(dt, current_player_position,
                                   world.get_voxel_space_data(),
                                   global_player_visibility_flag);
    }
    if (enemy_signals.enemy_is_killable && player_actions.attack) {
      enemy.disable();
    }
    if (enemy_signals.player_is_visible) {
      // if any enemy can see the player, we set to true
      global_player_visibility_flag = true;
    }
    if (enemy_signals.projectile_hit_player) {
      player.decrease_health(25);
    }
  }
  // furthermore, passing signals between the enemies is not convenient; here
  // we definitely need some kind of blackboarding or global state management
  hud.update(dt);

  // return codes
  if (player.check_health() <= 0) {
    // sneakily reset the health to allow restart
    player.reset_health();
    player.reset_position();
    return 1;
  }

  // Check if all enemies are disabled
  bool all_enemies_dead =
      std::all_of(enemies.begin(), enemies.end(), [](const Enemy &e) {
        return e.get_state() == EnemyState::DEAD;
      });

  if (all_enemies_dead) {
    player.reset_health();
    player.reset_position();
    // Reset all enemies
    for (auto &enemy : enemies) {
      enemy.reset();
    }
    return 2;
  }

  return 0;
}

void Game::draw() {
  BeginDrawing();
  ClearBackground(BLACK);

  BeginMode3D(player.get_camera());
  world.draw();
  player.draw(world.get_voxel_space_data());
  for (auto &enemy : enemies) {
    enemy.draw();
  }
  EndMode3D();

  hud.draw(player.get_position(), player.check_health());
  EndDrawing();
}

Game::~Game() { delete pather; }
