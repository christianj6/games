#include "app.h"
#include "raylib.h"

// TODO: change default game state to MAIN_MENU
App::App() : current_state(AppState::GAME), game() {}

void App::handle_input() {
  switch (current_state) {
  case AppState::MAIN_MENU:
    if (IsKeyPressed(KEY_ENTER)) {
      current_state = AppState::GAME;
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
      current_state = AppState::QUIT;
    }
    break;
  case AppState::GAME:
    if (IsKeyPressed(KEY_ESCAPE)) {
      // TODO: revert
      /*current_state = AppState::PAUSE;*/
      current_state = AppState::QUIT;
    }
    break;
  case AppState::PAUSE:
    if (IsKeyPressed(KEY_ESCAPE)) {
      current_state = AppState::GAME;
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

bool App::run() {
  handle_input();
  switch (current_state) {
  case AppState::MAIN_MENU:
    main_menu();
    break;
  case AppState::GAME:
    game.update();
    game.draw();
    break;
  case AppState::PAUSE:
    pause_menu();
    break;
  case AppState::QUIT:
    return false;
  }
  return true;
}
