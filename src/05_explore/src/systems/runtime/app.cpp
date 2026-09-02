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
  case ApplicationState::GAME: {
    if (!game_)
      game_ = std::make_unique<Game>();
    GameInfo info = game_->tick(debug);
    GameState s = game_->get_current_state();
    if (info.restart) {
      game_ = std::make_unique<Game>(); // fresh world
    } else if (info.quit_to_menu) {
      game_.reset();
      current_state = ApplicationState::MAIN_MENU;
    } else if ((s == GameState::WIN || s == GameState::LOSE) &&
               IsKeyPressed(KEY_ENTER)) {
      game_.reset();
      current_state = ApplicationState::MAIN_MENU;
    }
    break;
  }

  case ApplicationState::QUIT:
    run = false;
  }
  EndDrawing();

  return run;
}

void App::main_menu() {
  int cx = GetScreenWidth() / 2;
  int cy = GetScreenHeight() / 2;
  const char *title = "EXPLORE";
  int tw = MeasureText(title, 60);
  DrawText(title, cx - tw / 2, cy - 80, 60, BLACK);
  const char *prompt = "Press ENTER to start  -  ESC to quit";
  int pw = MeasureText(prompt, 24);
  DrawText(prompt, cx - pw / 2, cy + 20, 24, GRAY);
}
