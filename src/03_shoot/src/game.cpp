#include "game.h"
#include "raylib.h"
#include "raymath.h"

Game::Game()
    : world(), player(), hud(), game_over(false), game_won(false),
      game_over_timer(0.0f) {
  player.set_world(&world);
}

void Game::update() {
  float dt = GetFrameTime();

  if (!game_over && !game_won) {
    // main game loop
    player.handle_input();
    Vector3 current_player_position = player.update(dt);
    world.update(dt, current_player_position);

    // Check projectile-enemy collisions
    auto &projectiles = player.get_projectiles();
    const auto &enemies = world.get_enemies();

    for (size_t enemy_idx = 0; enemy_idx < enemies.size(); enemy_idx++) {
      const auto &enemy = enemies[enemy_idx];
      for (auto &projectile : projectiles) {
        if (!projectile.is_active())
          continue;

        float dist =
            Vector3Distance(projectile.get_position(), enemy->get_position());
        if (dist < 2.0f) { // Hit radius
          projectile.deactivate();
          world.remove_enemy(enemy_idx);
          break;
        }
      }
    }

    // Check for enemy collision
    for (const auto &enemy : world.get_enemies()) {
      float dist =
          Vector3Distance(current_player_position, enemy->get_position());
      if (dist < CATCH_DISTANCE) {
        game_over = true;
        break;
      }
    }

    // Check for win condition
    if (world.get_enemies().empty()) {
      game_won = true;
    }

    hud.update(dt);
  }
}

void Game::draw() {
  BeginDrawing();
  ClearBackground(BLACK);

  BeginMode3D(player.get_camera());
  world.draw();
  player.draw();
  EndMode3D();
  hud.draw();

  // in the future we can extract this kind of stuff into other abstractions or a message handling system
  if (game_over || game_won) {
    const char *text = game_won ? "YOU WIN!" : "GAME OVER";
    int fontSize = 60;
    int textWidth = MeasureText(text, fontSize);
    DrawText(text, GetScreenWidth() / 2 - textWidth / 2,
             GetScreenHeight() / 2 - fontSize / 2, fontSize, WHITE);

    // Add restart text below
    const char *restart_text = "Restart to Play Again";
    int restart_fontSize = 20;
    int restart_textWidth = MeasureText(restart_text, restart_fontSize);
    DrawText(restart_text, GetScreenWidth() / 2 - restart_textWidth / 2,
             GetScreenHeight() / 2 +
                 fontSize, // Position it below the GAME OVER text
             restart_fontSize, GRAY);
  }

  EndDrawing();
}
