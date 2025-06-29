#include "game.h"
#pragma once

enum class AppState { MAIN_MENU, GAME, PAUSE, QUIT };

class App {
public:
  App(bool = false);
  bool run();

private:
  void handle_input();
  void main_menu();
  void pause_menu();

  bool debug_mode_enabled;
  AppState current_state;
  Game game;
};
