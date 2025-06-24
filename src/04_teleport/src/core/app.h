#include "game.h"
#pragma once

enum class AppState { MAIN_MENU, GAME, PAUSE, QUIT };

class App {
public:
  App();
  void run();

private:
  void handle_input();
  void main_menu();
  void pause_menu();

  AppState current_state;
  Game game;
};
