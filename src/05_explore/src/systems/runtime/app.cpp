#include "app.h"
#include "raylib.h"

bool App::is_game_running() const {
  return current_state == ApplicationState::GAME && game_ != nullptr &&
         game_->get_current_state() == GameState::RUNNING;
}

bool App::run(bool debug) {
  BeginDrawing();
  ClearBackground(RAYWHITE);

  bool run = true;
  switch (current_state) {
  case ApplicationState::MAIN_MENU:
    if (IsKeyPressed(KEY_ENTER)) {
      current_state = ApplicationState::GAME;
    }
#ifndef PLATFORM_WEB
    if (IsKeyPressed(KEY_ESCAPE)) {
      current_state = ApplicationState::QUIT;
    }
#endif // web: there is no window to quit — closing the tab is the quit
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
  DrawText("EXPLORE", cx - MeasureText("EXPLORE", 60) / 2, cy - 80, 60, BLACK);
  const char *prompt = "Press ENTER to start  -  ESC to quit";
  DrawText(prompt, cx - MeasureText(prompt, 24) / 2, cy + 20, 24, GRAY);
}

void App::toggle_pause() {
  if (game_)
    game_->toggle_pause();
}
