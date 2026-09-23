#include "app.h"
#include "raylib.h"

namespace {
constexpr int kTitleFontSize = 60;
constexpr int kTitleYOffset = -80;
constexpr int kMenuPromptFontSize = 24;
constexpr int kMenuPromptYOffset = 20;
} // namespace

bool App::is_game_running() const {
  return current_state == ApplicationState::GAME && game_ != nullptr &&
         game_->get_current_state() == GameState::RUNNING;
}

bool App::run() {
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
    GameInfo info = game_->tick();
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

void App::main_menu() const {
  int cx = GetScreenWidth() / 2;
  int cy = GetScreenHeight() / 2;
  DrawText("EXPLORE", cx - MeasureText("EXPLORE", kTitleFontSize) / 2,
           cy + kTitleYOffset, kTitleFontSize, BLACK);
#ifdef PLATFORM_WEB
  // Web has no window to quit — closing the tab is the quit.
  const char *prompt = "Press ENTER to start";
#else
  const char *prompt = "Press ENTER to start  -  ESC to quit";
#endif
  DrawText(prompt, cx - MeasureText(prompt, kMenuPromptFontSize) / 2,
           cy + kMenuPromptYOffset, kMenuPromptFontSize, GRAY);
}

void App::toggle_pause() {
  if (game_)
    game_->toggle_pause();
}
