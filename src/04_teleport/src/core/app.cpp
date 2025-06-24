#include "app.h"
#include "raylib.h"

App::App() : current_state(AppState::MAIN_MENU), game() {}

void App::handle_input() {
  switch (current_state) {
  case AppState::MAIN_MENU:
    if (IsKeyPressed(KEY_ENTER)) {
      current_state = AppState::GAME;
    }
    break;
  case AppState::GAME:
    if (IsKeyPressed(KEY_SPACE)) {
      current_state = AppState::PAUSE;
    }
    break;
  case AppState::PAUSE:
    if (IsKeyPressed(KEY_SPACE)) {
      current_state = AppState::GAME;
    }
    break;
  case AppState::QUIT:
    break;
  }
}

void App::main_menu() {
  BeginDrawing();
  ClearBackground(WHITE);
  DrawText("main menu", GetScreenWidth() / 2, GetScreenHeight() / 2, 40, BLUE);
  EndDrawing();
}

void App::pause_menu() {
  BeginDrawing();
  ClearBackground(WHITE);
  DrawText("pause", GetScreenWidth() / 2, GetScreenHeight() / 2, 40, RED);
  EndDrawing();
}

void App::run() {
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
    // TODO: RL_FREE(transforms);
    return;
  }
}
