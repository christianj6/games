#pragma once
#include "game.h"

enum class ApplicationState { MAIN_MENU, GAME, QUIT };

class App {
public:
  bool run(bool);

private:
  ApplicationState current_state = ApplicationState::MAIN_MENU;
  std::unique_ptr<Game> game_; // built lazily on first ENTER
  void main_menu();
};
