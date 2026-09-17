#include "game.h"
#pragma once

enum class AppState { MAIN_MENU, GAME_PLAY, GAME_OVER, GAME_WIN, PAUSE, QUIT };

class App {
public:
  App(bool = false);
  bool run();
  AppState state() const { return current_state; }
  // Web only: pointer lock lost mid-gameplay (browser exited via ESC) acts
  // as the pause key, since the browser swallows that keydown.
  void pause() { current_state = AppState::PAUSE; }

private:
  void handle_input();
  void main_menu();
  void pause_menu();
  void game_over();
  void game_win();

  bool debug_mode_enabled;
  AppState current_state;
  Game game;
};
