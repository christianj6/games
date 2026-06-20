#include "app.h"
#include "raylib.h"

bool App::run(bool debug) {
  BeginDrawing();
  ClearBackground(RAYWHITE);

  bool run = true;
  switch (current_state) {
  case ApplicationState::MAIN_MENU:
    if (IsKeyPressed(KEY_ENTER)) {
      current_state = ApplicationState::GAME;
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
      current_state = ApplicationState::QUIT;
    }
    main_menu();
    break;
  case ApplicationState::GAME:
    // TODO: consider using GameInfo return to streamline this condition
    if (game.get_current_state() == GameState::PAUSED &&
        (IsKeyPressed(KEY_Q) ||
         IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_LEFT))) {
      current_state = ApplicationState::QUIT;
    } else {
      game.tick(debug);
    }
    break;
  case ApplicationState::QUIT:
    run = false;
  }
  EndDrawing();

  return run;
}

void App::main_menu() {
  DrawText("MAIN MENU", GetScreenWidth() / 2, GetScreenHeight() / 2, 40, BLACK);
}
