#include "app.h"
#include "raylib.h"

App::App(bool debug_mode_enabled)
    : current_state(debug_mode_enabled ? AppState::GAME_PLAY
                                       : AppState::MAIN_MENU),
      game(debug_mode_enabled), debug_mode_enabled(debug_mode_enabled) {}

void App::handle_input() {
  switch (current_state) {
  case AppState::MAIN_MENU:
    if (IsKeyPressed(KEY_ENTER)) {
      current_state = AppState::GAME_PLAY;
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
      current_state = AppState::QUIT;
    }
    break;
  case AppState::GAME_PLAY:
    if (IsKeyPressed(KEY_ESCAPE)) {
      if (debug_mode_enabled) {
        current_state = AppState::QUIT;
      } else {
        current_state = AppState::PAUSE;
      }
    }
    break;
  case AppState::GAME_OVER:
    if (IsKeyPressed(KEY_ENTER)) {
      current_state = AppState::GAME_PLAY;
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
      current_state = AppState::QUIT;
    }
    break;
  case AppState::GAME_WIN:
    if (IsKeyPressed(KEY_ENTER)) {
      current_state = AppState::GAME_PLAY;
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
      current_state = AppState::QUIT;
    }
    break;
  case AppState::PAUSE:
    if (IsKeyPressed(KEY_ESCAPE)) {
      current_state = AppState::GAME_PLAY;
    }
    if (IsKeyPressed(KEY_ENTER)) {
      current_state = AppState::QUIT;
    }
    break;
  case AppState::QUIT:
    break;
  }
}

void App::main_menu() {
  BeginDrawing();
  ClearBackground(WHITE);
  // TODO
  DrawText("main menu", GetScreenWidth() / 2, GetScreenHeight() / 2, 40, BLUE);
  DrawText("press ENTER to play the game", GetScreenWidth() / 2,
           GetScreenHeight() / 2 + 50, 40, RED);
  DrawText("press ESCAPE to quit", GetScreenWidth() / 2,
           GetScreenHeight() / 2 + 50 * 2, 40, RED);
  EndDrawing();
}

void App::pause_menu() {
  BeginDrawing();
  ClearBackground(WHITE);
  // TODO
  DrawText("pause", GetScreenWidth() / 2, GetScreenHeight() / 2, 40, GREEN);
  DrawText("press ENTER to quit", GetScreenWidth() / 2,
           GetScreenHeight() / 2 + 50, 40, RED);
  EndDrawing();
}

void App::game_over() {
  ClearBackground(WHITE);
  // TODO
  DrawText("loser", GetScreenWidth() / 2, GetScreenHeight() / 2, 40, RED);
  DrawText("press ENTER to play again", GetScreenWidth() / 2,
           GetScreenHeight() / 2 + 50, 40, RED);
  DrawText("press ESCAPE to quit", GetScreenWidth() / 2,
           GetScreenHeight() / 2 + 50 * 2, 40, RED);
  EndDrawing();
}

void App::game_win() {
  BeginDrawing();
  ClearBackground(WHITE);
  // TODO
  DrawText("winner", GetScreenWidth() / 2, GetScreenHeight() / 2, 40, GREEN);
  DrawText("press ENTER to play again", GetScreenWidth() / 2,
           GetScreenHeight() / 2 + 50, 40, RED);
  DrawText("press ESCAPE to quit", GetScreenWidth() / 2,
           GetScreenHeight() / 2 + 50 * 2, 40, RED);
  EndDrawing();
}

bool App::run() {
  handle_input();
  int game_status = 0;
  switch (current_state) {
  case AppState::MAIN_MENU:
    main_menu();
    break;
  case AppState::GAME_PLAY:
    game_status = game.update();
    if (game_status == 1) {
      current_state = AppState::GAME_OVER;
    } else if (game_status == 2) {
      current_state = AppState::GAME_WIN;
    }
    game.draw();
    break;
  case AppState::GAME_OVER:
    game_over();
    break;
  case AppState::GAME_WIN:
    game_win();
    break;
  case AppState::PAUSE:
    pause_menu();
    break;
  case AppState::QUIT:
    return false;
  }
  return true;
}
