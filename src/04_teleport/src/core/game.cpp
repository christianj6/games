#include "game.h"
#include "raylib.h"
#include "utils/resource_dir.h"

Game::Game(bool debug_mode_enabled) : hud(debug_mode_enabled), world(), map() {
  SearchAndSetResourceDir("resources");
  // try to place 20 enemies
  const int n_enemies = 20;
  pather = new micropather::MicroPather(&map, 250);
  // because world is static we can set the world data once
  map.set_vector_space_data(world.get_voxel_space_data());
  for (int i = 0; i < n_enemies; i++) {
    Enemy enemy;
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

void Game::update() {
  // noticing that i want to pass different signals between the actors
  // and resort to hacks or unmanaged data passing to do this; in a next
  // iteration we likely need to extend the gameplay layer to handle this
  float dt = GetFrameTime();
  bool trying_to_kill = player.update(dt, world.get_voxel_space_data());
  world.update(dt, player.get_camera());
  Vector3 current_player_position = player.get_position();
  for (auto &enemy : enemies) {
    bool is_killable = enemy.update(dt, current_player_position,
                                    world.get_voxel_space_data(), pather);
    if (is_killable && trying_to_kill) {
      enemy.disable();
    }
  }
  hud.update(dt);
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

  hud.draw(player.get_position());
  EndDrawing();
}

Game::~Game() { delete pather; }
